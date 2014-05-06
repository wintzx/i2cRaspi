/*--------------------------------------------------------
 *
 *--------------------------------------------------------
 * Project    : I2C
 * Sub-Project: i2cTest.cpp
 *
 * Copyright Patrick DELVENNE and ProcessUX 2014
 *--------------------------------------------------------
 */

#include "LcdDisplay/LcdDisplay.h"

#include <exception>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

int getMyIP(int fd, char* ifname,  u_int32_t * ipaddr) ;

//---------------------------------------------------
int main (void){
//---------------------------------------------------

	struct tm *t ;
	time_t tim ;

	int sock = socket(PF_INET, SOCK_STREAM,0);
	if(sock == -1) {
		perror("main: socket \n" );
		//exit(1);
	}
	u_int32_t nip;
	getMyIP(sock,"eth0",&nip);
	struct in_addr in;
	in.s_addr = nip;
	char * cip =  inet_ntoa( in );


	bool isRes = true;
	bool isLcdInitialized = false;
	unsigned char szAdresse = 0x3F;

	lcdDisplay lcd(szAdresse);

	try{
		lcd.init();
		isLcdInitialized = true;
	}catch(std::exception const& e){
		printf("%s\n",e.what());
	}
	if(true == isLcdInitialized){
		lcd.cls();
		char szL2[K_LCD_MAX_CHAR_PER_LINE];
		snprintf(szL2,20,"IP=%s",cip);
		char szL3[K_LCD_MAX_CHAR_PER_LINE];
		try{
			//lcd.displayStringAtLine(szL1,1);
			lcd.displayStringAtPosition(szL2,2);
			lcd.displayStringAtPosition("http:\/\/www.wintzx.fr",4);
			lcd.displayStringAtPosition("WINTZX",1,7);
			lcd.setCursorAtPosition(1,8,true, true);
		}catch(std::exception const& e){
			printf(e.what());
		}

		do{
			tim = time (NULL) ;
			t = localtime (&tim) ;
			snprintf (szL3, 20 ,"%02d:%02d:%02d", t->tm_hour, t->tm_min, t->tm_sec) ;
			try{
				lcd.displayStringAtPosition(szL3,3);
			}catch(std::exception const& e){
				isRes = false;
			}
			usleep(5000);
		}while (isRes == true);
	}
}

//---------------------------------------------------
int getMyIP(int fd, char* ifname,  u_int32_t * ipaddr) {
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
		perror("getMyIP: ioctl" );
	//exit(-1);
	}
	memcpy(ipaddr,&ifr.ifr_addr.sa_data[2],4);
	return 0;
}
