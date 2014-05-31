/*--------------------------------------------------------
 *
 *--------------------------------------------------------
 * Project    : I2C
 * Sub-Project: i2cTest.cpp
 *
 * Copyright Patrick DELVENNE and ProcessUX 2014
 *--------------------------------------------------------
 */

#include <exception>
#include <csignal>
#include <stdexcept>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#include "LcdDisplay/LcdDisplay.h"
#include "Ds1621/Ds1621.h"

// Devices
const unsigned char K_I2C_LCD_ADDRES            = 0x3F;
const unsigned char K_I2C_DS1621_OUT_ADDRES     = 0x48;
const unsigned char K_I2C_DS1621_IN_ADDRES      = 0x49;

typedef struct _TS_i2cConfig{
	FILE* pTemperatureFile;
	char *pEth0Ip;
	LcdDisplay *pLcdDevice;
	Ds1621 *pInsideDS1621Device;
	Ds1621 *pOutsideDS1621Device;
	pthread_mutex_t i2cMutex;
	pthread_t threadTemperatureID;
	pthread_t threadFileID;
	unsigned int nFilePeriod;
	bool isSignalCaptured;
	bool isClsOnly;
	bool isDisplayIpOnly;
	bool isInsideTL;
	bool isInsideTH;
	bool isOutsideTL;
	bool isOutsideTH;
	bool isLcdEnabled;
	bool isInsideDs1621Enabled;
	bool isOutsideDs1621Enabled;
	bool isInsideDs1621Ready;
	bool isOutsideDs1621Ready;
	bool isLcdReady;
	bool isFileEnabled;
	float fInsideTL;
	float fInsideTH;
	float fOutsideTL;
	float fOutsideTH;
	float fInsideTemp;
	float fOutsideTemp;


	_TS_i2cConfig():
	pTemperatureFile(NULL),
	pLcdDevice(NULL),
	pInsideDS1621Device(NULL),
	pOutsideDS1621Device(NULL),
	nFilePeriod(60),
	isSignalCaptured(false),
	isClsOnly(false),
	isDisplayIpOnly(false),
	isInsideTL(false),
	isInsideTH(false),
	isOutsideTL(false),
	isOutsideTH(false),
	isLcdEnabled(true),
	isInsideDs1621Enabled(true),
	isOutsideDs1621Enabled(true),
	isInsideDs1621Ready(false),
	isOutsideDs1621Ready(false),
	isLcdReady(false),
	isFileEnabled(false)
	{}

}TS_i2cConfig;

int getMyIP(int nFd, const char* szIfname,  u_int32_t *ipaddr) ;
FILE* createFileForTemp();
void parseArg(int argc, char *argv[],TS_i2cConfig& i2cTestConfig);
void cleanOnExit(TS_i2cConfig* pCfg);
void displaySplash(TS_i2cConfig& i2cTestConfig);
void sigHandler(int nSignum);

// Ugly global variable.
TS_i2cConfig i2cTestConfig;

//---------------------------------------------------
static void* threadTemperatureProcess(void *pConfig){
//---------------------------------------------------
	TS_i2cConfig *pCfg = (TS_i2cConfig *)pConfig;
	char szL1[K_LCD_MAX_CHAR_PER_LINE];

	printf("Temperature thread is starting...\n");
	do{
		try{
			if(true == pCfg->isInsideDs1621Ready){
				// Lock I2C ACCES
				// ---> START OF MUTEX SECTION
				pthread_mutex_lock(&pCfg->i2cMutex);
				pCfg->fInsideTemp = pCfg->pInsideDS1621Device->getLRTemp();
				if(true == i2cTestConfig.isLcdReady){
					//printf("Tin=%2.2f\n",pCfg->fInsideTemp);
					snprintf(szL1,K_LCD_MAX_CHAR_PER_LINE,"Tin=%2.2f",pCfg->fInsideTemp);
					pCfg->pLcdDevice->displayStringAtPosition(szL1,1,11);
				}
				// unLock I2C ACCES
				pthread_mutex_unlock(&pCfg->i2cMutex);
				// <-- END OF MUTEX SECTION
			}

			// Sleep 1 second
			sleep(1);

			if(true == pCfg->isOutsideDs1621Ready){
				// Lock I2C ACCES
				// ---> START OF MUTEX SECTION
				pthread_mutex_lock(&pCfg->i2cMutex);
				pCfg->fOutsideTemp = pCfg->pOutsideDS1621Device->getLRTemp();
				if(true == i2cTestConfig.isLcdReady){
					//printf("Tout=%2.2f\n",pCfg->fOutsideTemp);
					snprintf(szL1,K_LCD_MAX_CHAR_PER_LINE,"Tout=%2.2f",pCfg->fOutsideTemp);
					pCfg->pLcdDevice->displayStringAtPosition(szL1,1,0);
				}
				// unLock I2C ACCES
				pthread_mutex_unlock(&pCfg->i2cMutex);
				// <-- END OF MUTEX SECTION
			}
		}catch(std::exception const& e){
			printf("[DS1621] %s\n",e.what());
			cleanOnExit(pCfg);
		}

		// Sleep 1 second
		sleep(1);

	}while(false == pCfg->isSignalCaptured);
	printf("Temperature thread is ending...\n");
	return NULL;
}

//---------------------------------------------------
static void* threadFileProcess(void *pConfig){
//---------------------------------------------------
	unsigned int nIndex = 0;
	struct tm *t ;
	time_t tim ;
	TS_i2cConfig *pCfg = (TS_i2cConfig *)pConfig;

	printf("File thread is starting...\n");
	do{
		if(nIndex % pCfg->nFilePeriod == 0){
			if(NULL != pCfg->pTemperatureFile){
				tim = time (NULL) ;
				t = localtime (&tim) ;
				// Lock I2C ACCES
				// ---> START OF MUTEX SECTION
				pthread_mutex_lock(&pCfg->i2cMutex);
				fprintf(pCfg->pTemperatureFile, "%02d:%02d:%02d;%2.2f;%2.2f\n", t->tm_hour, t->tm_min, t->tm_sec,pCfg->fInsideTemp,pCfg->fOutsideTemp);
				//printf("%02d:%02d:%02d;%2.2f;%2.2f\n", t->tm_hour, t->tm_min, t->tm_sec,pCfg->fInsideTemp,pCfg->fOutsideTemp);
				// unLock I2C ACCES
				pthread_mutex_unlock(&pCfg->i2cMutex);
				// <-- END OF MUTEX SECTION

				fflush(pCfg->pTemperatureFile);
			}
		}
		nIndex++;

		// Sleep 1 second
		sleep(1);

	}while(false == pCfg->isSignalCaptured);
	printf("File thread is ending...\n");
	return NULL;
}

//---------------------------------------------------
void sigHandler(int nSignum){
//---------------------------------------------------
	printf("[WARNING] signal %d captured !\n",nSignum);
	cleanOnExit(&i2cTestConfig);

	//signal(nSignum, SIG_DFL);
	//if(SIGINT == nSignum){
	//  kill(getpid(), SIGINT);
	//}
}

//---------------------------------------------------
int main (int argc, char *argv[]){
// gpio load i2c 100
// i2cTest -cls clear screen and exit
// i2cTest -ipd display IP and exit
// i2cTest -tli xx.yy set low threshold for inside device
// i2cTest -thi xx.yy set high threshold for inside device
// i2cTest -tlo xx.yy set low threshold for outside device
// i2cTest -tho xx.yy set high threshold for outside device
// i2cTest -nolcd disable LCD device
// i2cTest -noit disable inside device
// i2cTest -noot disable outside device
// i2cTest -file xx record temperature in a file every xx seconds

//---------------------------------------------------

	// Register signal and handler
	signal(SIGINT,sigHandler);
	signal(SIGTERM,sigHandler);

	i2cTestConfig.i2cMutex = PTHREAD_MUTEX_INITIALIZER;

	struct tm *t ;
	time_t tim ;

	int nSock = socket(PF_INET, SOCK_STREAM,0);
	if(-1 == nSock) {
		printf("[ERROR] socket could not be created !\n" );
		return -1;
	}

	u_int32_t nEth0ip;;

	getMyIP(nSock,"eth0",&nEth0ip);
	struct in_addr inEth0;
	inEth0.s_addr = nEth0ip;
	i2cTestConfig.pEth0Ip = inet_ntoa( inEth0 );


	float fInsideTemp = 0.0;
	float fOutsideTemp =0.0;

	// Parse programm arguments
	parseArg(argc,argv,i2cTestConfig);

	// Devices creation and initialization
	if(true == i2cTestConfig.isLcdEnabled){
		i2cTestConfig.pLcdDevice            = new LcdDisplay(K_I2C_LCD_ADDRES);
		if(NULL != i2cTestConfig.pLcdDevice){
			i2cTestConfig.pLcdDevice->init();
			if (true == i2cTestConfig.pLcdDevice->isDeviceUp()){
				i2cTestConfig.isLcdReady = true;

				// We just wanted to clean screen , let's exit
				if(true == i2cTestConfig.isClsOnly){
					i2cTestConfig.pLcdDevice->cls();
					cleanOnExit(&i2cTestConfig);
					return 0;
				}

				// We just wanted to show IP on the screen , let's exit
				if(true == i2cTestConfig.isDisplayIpOnly){
					displaySplash(i2cTestConfig);
					cleanOnExit(&i2cTestConfig);
					return 0;
				}
			}
		}
	}

	if(true == i2cTestConfig.isInsideDs1621Enabled){
		i2cTestConfig.pInsideDS1621Device   = new Ds1621(K_I2C_DS1621_IN_ADDRES);
		if(NULL != i2cTestConfig.pInsideDS1621Device){
			i2cTestConfig.pInsideDS1621Device->init();
			if (true == i2cTestConfig.pInsideDS1621Device->isDeviceUp()){
				i2cTestConfig.isInsideDs1621Ready = true;
			}
		}
	}

	if(true == i2cTestConfig.isOutsideDs1621Enabled){
		i2cTestConfig.pOutsideDS1621Device  = new Ds1621(K_I2C_DS1621_OUT_ADDRES);
		if(NULL != i2cTestConfig.pOutsideDS1621Device){
			i2cTestConfig.pOutsideDS1621Device->init();
			if (true == i2cTestConfig.pOutsideDS1621Device->isDeviceUp()){
				i2cTestConfig.isOutsideDs1621Ready = true;
			}
		}
	}

	// Here come serious things
	if(true == i2cTestConfig.isInsideDs1621Ready){
		try{
			if(true == i2cTestConfig.isInsideTH){
				printf("Setting Inside High temperature threshold to %2.2f\n",i2cTestConfig.fInsideTH);
				i2cTestConfig.pInsideDS1621Device->setThresholdTemp(i2cTestConfig.fInsideTH,false);
			}
			if(true == i2cTestConfig.isInsideTL){
				printf("Setting Inside Low temperature threshold to %2.2f\n",i2cTestConfig.fInsideTL);
				i2cTestConfig.pInsideDS1621Device->setThresholdTemp(i2cTestConfig.fInsideTL,true);
			}
			// Display configuration
			i2cTestConfig.pInsideDS1621Device->displayConfig();
			fInsideTemp = i2cTestConfig.pInsideDS1621Device->getHRTemp();
			printf("Temperature inside = %2.2f\n",fInsideTemp);
		}catch(std::exception const& e){
			printf("[InDS1621] %s\n",e.what());
			cleanOnExit(&i2cTestConfig);
		}
	}

	if(true == i2cTestConfig.isOutsideDs1621Ready){
		try{
			if(true == i2cTestConfig.isOutsideTH){
				printf("Setting Outside High temperature threshold to %2.2f\n",i2cTestConfig.fOutsideTH);
				i2cTestConfig.pOutsideDS1621Device->setThresholdTemp(i2cTestConfig.fOutsideTH,false);
			}
			if(true == i2cTestConfig.isOutsideTL){
				printf("Setting Outside Low temperature threshold to %2.2f\n",i2cTestConfig.fOutsideTL);
				i2cTestConfig.pOutsideDS1621Device->setThresholdTemp(i2cTestConfig.fOutsideTL,true);
			}
			// Display configuration
			i2cTestConfig.pOutsideDS1621Device->displayConfig();
			fOutsideTemp = i2cTestConfig.pOutsideDS1621Device->getHRTemp();
			printf("Temperature outside = %2.2f\n",fOutsideTemp);
		}catch(std::exception const& e){
			printf("[OutDS1621] %s\n",e.what());
			cleanOnExit(&i2cTestConfig);
		}
	}

	char szL2[K_LCD_MAX_CHAR_PER_LINE];
	char szL3[K_LCD_MAX_CHAR_PER_LINE];
	snprintf(szL2,K_LCD_MAX_CHAR_PER_LINE,"IP=%s",i2cTestConfig.pEth0Ip);

	if(true == i2cTestConfig.isLcdReady){
		try{
			i2cTestConfig.pLcdDevice->cls();
			// Display line #2
			i2cTestConfig.pLcdDevice->displayStringAtPosition(szL2,2);
			// Display line #4
			i2cTestConfig.pLcdDevice->displayStringAtPosition("http:\/\/www.wintzx.fr",4);
			// Display cursor line #4 at position 1 and make it blink
			i2cTestConfig.pLcdDevice->setCursorAtPosition(4,1,true, true);
		}catch(std::exception const& e){
			printf("[LCD] %s\n",e.what());
			cleanOnExit(&i2cTestConfig);
		}
	}

	pthread_create(&i2cTestConfig.threadTemperatureID, NULL, threadTemperatureProcess,  &i2cTestConfig);
	if((true == i2cTestConfig.isFileEnabled) &&
	  ((true == i2cTestConfig.isInsideDs1621Ready) || (true == i2cTestConfig.isOutsideDs1621Ready))){
		i2cTestConfig.pTemperatureFile = createFileForTemp();
		pthread_create(&i2cTestConfig.threadFileID, NULL, threadFileProcess,  &i2cTestConfig);
	}

	do{
		if(true == i2cTestConfig.isLcdReady){
			tim = time (NULL) ;
			t = localtime (&tim) ;
			snprintf (szL3, K_LCD_MAX_CHAR_PER_LINE ,"%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec) ;
			try{
					// Lock I2C ACCES
					// ---> START OF MUTEX SECTION
					pthread_mutex_lock(&i2cTestConfig.i2cMutex);
					i2cTestConfig.pLcdDevice->displayStringAtPosition(szL3,3);
					// unLock I2C ACCES
					pthread_mutex_unlock(&i2cTestConfig.i2cMutex);
					// <-- END OF MUTEX SECTION
			}catch(std::exception const& e){
				printf("[LCD] %s\n",e.what());
				cleanOnExit(&i2cTestConfig);
			}
		}
		// Sleep for 500ms
		// The useconds argument must be less than 1,000,000.
		usleep(500000);
	}while (false == i2cTestConfig.isSignalCaptured);

	cleanOnExit(&i2cTestConfig);
	printf("BYE..\n");
}

//---------------------------------------------------
int getMyIP(int fd, const char* ifname,  u_int32_t * ipaddr) {
//---------------------------------------------------
  /* fd: opened file descriptor
   * ifname: if name eg: "eth0"
   */
	#define IFNAMSIZ 16
	struct ifreq{
		char    ifr_name[IFNAMSIZ];
		struct  sockaddr ifr_addr;
	};
	struct ifreq ifr;
	memcpy(ifr.ifr_name,ifname,IFNAMSIZ);
	if(ioctl(fd,SIOCGIFADDR,&ifr) == -1){
		printf("[ERROR] ioctl!\n" );
		return -1;
	}
	memcpy(ipaddr,&ifr.ifr_addr.sa_data[2],4);
	return 0;
}

//---------------------------------------------------
FILE* createFileForTemp(){
//---------------------------------------------------
	struct tm *t ;
	time_t tim ;
	FILE* pFile = NULL;
	char szFileName[K_LCD_MAX_CHAR_PER_LINE];
	tim = time (NULL) ;
	t = localtime (&tim) ;
	snprintf (szFileName, K_LCD_MAX_CHAR_PER_LINE ,"Temp_20%02d%02d%02d.txt", t->tm_year, t->tm_hour, t->tm_min) ;
	pFile = fopen(szFileName, "w+");
	if(NULL == pFile){
		throw std::runtime_error("[Error] File creation error");
	}
	return pFile;
}

//---------------------------------------------------
void parseArg(int argc, char *argv[],TS_i2cConfig& i2cTestConfig){
//---------------------------------------------------
	for(int nIndex = 1; nIndex < argc; nIndex++){
		if(0 == strcmp(argv[nIndex],"-nolcd")){
			i2cTestConfig.isLcdEnabled = false;
			printf("Disabling LCD device\n");
		}
		if(0 == strcmp(argv[nIndex],"-noit")){
			i2cTestConfig.isInsideDs1621Enabled = false;
			printf("Disabling inside device\n");
		}
		if(0 == strcmp(argv[nIndex],"-noot")){
			i2cTestConfig.isOutsideDs1621Enabled = false;
			printf("Disabling outside device\n");
		}
		if(0 == strcmp(argv[nIndex],"-thi")){
			// Set High temperature threshold
			i2cTestConfig.isInsideTH = true;
			sscanf(argv[nIndex + 1],"%f",&i2cTestConfig.fInsideTH);
		}
		if(0 == strcmp(argv[nIndex],"-tli")){
			// Set High temperature threshold
			i2cTestConfig.isInsideTL = true;
			sscanf(argv[nIndex + 1],"%f",&i2cTestConfig.fInsideTL);
		}
		if(0 == strcmp(argv[nIndex],"-tho")){
			// Set High temperature threshold
			i2cTestConfig.isOutsideTH = true;
			sscanf(argv[nIndex + 1],"%f",&i2cTestConfig.fOutsideTH);
		}
		if(0 == strcmp(argv[nIndex],"-tlo")){
			// Set High temperature threshold
			i2cTestConfig.isOutsideTL = true;
			sscanf(argv[nIndex + 1],"%f",&i2cTestConfig.fOutsideTL);
		}
		if(0 == strcmp(argv[nIndex],"-ipd")){
			i2cTestConfig.isDisplayIpOnly = true;
		}
		if(0 == strcmp(argv[nIndex],"-cls")){
			i2cTestConfig.isClsOnly = true;
		}
		if(0 == strcmp(argv[nIndex],"-file")){
			i2cTestConfig.isFileEnabled = true;
			sscanf(argv[nIndex + 1],"%ud",&i2cTestConfig.nFilePeriod);
			printf("Enabling file with %ds period\n",i2cTestConfig.nFilePeriod);
		}
	}
}
//---------------------------------------------------
void displaySplash(TS_i2cConfig& i2cTestConfig){
//---------------------------------------------------
	char szL2[K_LCD_MAX_CHAR_PER_LINE];
	if(NULL != i2cTestConfig.pLcdDevice){
		i2cTestConfig.pLcdDevice->displayStringAtPosition("*** RASPI READY! ***",1);
		snprintf(szL2,K_LCD_MAX_CHAR_PER_LINE,"IP=%s",i2cTestConfig.pEth0Ip);
		i2cTestConfig.pLcdDevice->displayStringAtPosition(szL2,2);
	}
}

//---------------------------------------------------
void cleanOnExit(TS_i2cConfig *pCfg){
//---------------------------------------------------
	// Spring cleaning

	pCfg->isSignalCaptured = true;

	// unLock I2C ACCES
	pthread_mutex_unlock(&pCfg->i2cMutex);

	void *result = NULL;
	// Wait for thread ending.
	pthread_join(pCfg->threadTemperatureID, &result);
	if(NULL != pCfg->pTemperatureFile){
		pthread_join(pCfg->threadFileID, &result);
	}

	if(NULL != pCfg->pTemperatureFile){
		fclose(pCfg->pTemperatureFile);
		pCfg->pTemperatureFile = NULL;
	}

	if(NULL != pCfg->pInsideDS1621Device){
		delete pCfg->pInsideDS1621Device;
		pCfg->pInsideDS1621Device = NULL;
	}

	if(NULL != pCfg->pOutsideDS1621Device){
		delete pCfg->pOutsideDS1621Device;
		pCfg->pOutsideDS1621Device = NULL;
	}

	if(NULL != pCfg->pLcdDevice){
		delete pCfg->pLcdDevice;
		pCfg->pLcdDevice = NULL;
	}
	pthread_mutex_destroy(&pCfg->i2cMutex);
}
