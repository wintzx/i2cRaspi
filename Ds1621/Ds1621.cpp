/*--------------------------------------------------------
 *
 *--------------------------------------------------------
 * Project    : I2C
 * Sub-Project: Ds1621.cpp
 *
 * Copyright Patrick DELVENNE and ProcessUX 2014
 *--------------------------------------------------------
 */

#include <wiringPiI2C.h>
#include <exception>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Ds1621.h"

//---------------------------------------------------
/**
  * Constructor
  * @param szAddres is the I2C addres of the device
*/
//---------------------------------------------------
Ds1621::Ds1621(unsigned char szAddres){
	m_szAddres = szAddres;
	m_isDeviceInitialized = false;
}

//---------------------------------------------------
/**
  * Destructor
*/
//---------------------------------------------------
Ds1621::~Ds1621(){
}

//---------------------------------------------------
/**
  * init : Init DS1621
  *
*/
//---------------------------------------------------
void Ds1621::init(){
	// Setup the device
	m_nDeviceFD = wiringPiI2CSetup (m_szAddres) ;
	if( -1 != m_nDeviceFD){
		try{
			setConfig(getConfig());
			m_isDeviceInitialized = true;
		}catch(std::exception const& e){
			printf("[DS1621] %s\n",e.what());
		}
	}
}

//---------------------------------------------------
/**
  * getLRTemp : Get the temperature with a low resolution (0.5C)
  *
  * @return the temperature
*/
//---------------------------------------------------
float Ds1621::getLRTemp(){
	int nTemp;

	// Sanity check
	M_F_DS1621_IS_DEVICE_UP

	// Start convert
	startStopConvert(false);
	nTemp = (signed int)wiringPiI2CReadReg16(m_nDeviceFD,K_DS1621_READ_TEMP);
	// Format temperature
	return formatTemp(nTemp);
}

//---------------------------------------------------
/**
  * getHRTemp : Get the temperature with a high resolution (0.01C)
  *
  * @return the temperature
*/
//---------------------------------------------------
float Ds1621::getHRTemp(){
	signed char szTemp;
	unsigned char szConfig = getConfig();
	signed char szCountRemain;
	signed char szCountPerC;
	signed int nIfract,nITemp;
	bool wasChanged = false;

	// Sanity check
	M_F_DS1621_IS_DEVICE_UP

	// Force 1Shot if needed
	if(false == isOneShot()){
		// Set 1SHOT bit
		setConfig(szConfig | K_DS1621_1SHOT_CONFIG);
		// Stop convert
		startStopConvert(true);
		wasChanged = true;
	}

	// Start convert
	startStopConvert(false);
	// Wait end of conversion
	waitEndOfConversion();

	szTemp          = (signed char)wiringPiI2CReadReg8(m_nDeviceFD,K_DS1621_READ_TEMP);
	szCountRemain   = (signed char)wiringPiI2CReadReg16(m_nDeviceFD,K_DS1621_READ_COUNTER);
	szCountPerC     = (signed char)wiringPiI2CReadReg16(m_nDeviceFD,K_DS1621_READ_SLOPE);

	// From DS1621 DataSheet
	// Temperature = TempRead - 0.25 + ((szCountPerC - szCountRemain) / szCountPerC)

	nIfract = ((signed int)(szCountPerC - szCountRemain) * 100) / 25;


	// Negative ?
	if(szTemp < 0) { // -
		nITemp = (signed char)szTemp * 100 + nIfract;
	}else {
		nITemp = (unsigned char)szTemp * 100 + nIfract;
	}

	// Restore config if needed
	if(true == wasChanged){
		setConfig(szConfig);
		// Start convert
		startStopConvert(false);
	}

	return (float)nITemp / 100.0;
}

//---------------------------------------------------
/**
  * isTHF : read the THF
  *
  * @return true if the DS1621 has ever been subjected to temperatures above TH, false otherwise
  *
*/
//---------------------------------------------------
bool Ds1621::isTHF(void){
	// Sanity check
	M_B_DS1621_IS_DEVICE_UP

	unsigned char szConfig = getConfig();
	return (szConfig & K_DS1621_THF_CONFIG) == K_DS1621_THF_CONFIG;
}

//---------------------------------------------------
/**
  * isTLF : read the TLF
  *
  * @return true if the DS1621 has ever been subjected to temperatures below TL, false otherwise
  *
*/
//---------------------------------------------------
bool Ds1621::isTLF(void){
	// Sanity check
	M_B_DS1621_IS_DEVICE_UP

	unsigned char szConfig = getConfig();
	return (szConfig & K_DS1621_TLF_CONFIG) == K_DS1621_TLF_CONFIG;
}

//---------------------------------------------------
/**
  * isOneShot : read the 1SHOT
  *
  * @return true if the DS1621 is in one shot mode, false otherwise
  *
*/
//---------------------------------------------------
bool Ds1621::isOneShot(void){
	// Sanity check
	M_B_DS1621_IS_DEVICE_UP

	unsigned char szConfig = getConfig();
	return (szConfig & K_DS1621_1SHOT_CONFIG) == K_DS1621_1SHOT_CONFIG;
}

//---------------------------------------------------
/**
  * setThresholdTemp : Set the low (TL) or high (TH) threshold temperature
  *
  * @param fTemp is the temperature to set
  * @param isLow if true set the low threshold, if false set the high
  *
  * @return the temperature that was set
*/
//---------------------------------------------------
float Ds1621::setThresholdTemp(float fTemp,bool isLow){
	unsigned char szCmd = K_DS1621_ACCES_TH;
	signed int nTemp;

	// Sanity check
	M_F_DS1621_IS_DEVICE_UP

	checkThresholdTemperatureRange(fTemp);

	// Format temperature
	if(fTemp >= 0.0) {
		// LSB
		nTemp = (unsigned char)fTemp;
		float r = fTemp - (unsigned int)fTemp;
		if(r >= 0.5){
			// Set bit 7 of MSB
			nTemp |= (0x01 << 15);
		}
	}else {
		// LSB
		nTemp = ((signed char)fTemp & 0x00FF);
		float r = -1 *(fTemp - (signed int)fTemp);
		if(r >= 0.5){
			// Set bit 7 of MSB
			nTemp |= (0x01 << 15);
		}
	}
	if(true == isLow){
		szCmd = K_DS1621_ACCES_TL;
	}
	writeRegister16bitsi2c(szCmd,nTemp);
	return fTemp;
}

//---------------------------------------------------
/**
  * getThresholdTemp : Get the low (TL) or high (TH) threshold temperature
  *
  * @param isLow if true get the low threshold, if false get the high
  *
  * @return the desired threshold temperature
*/
//---------------------------------------------------
float Ds1621::getThresholdTemp(bool isLow){
	signed int nTemp;
	unsigned char szCmd = K_DS1621_ACCES_TH;

	// Sanity check
	M_F_DS1621_IS_DEVICE_UP

	if(true == isLow){
		szCmd = K_DS1621_ACCES_TL;
	}
	nTemp = (signed int)wiringPiI2CReadReg16(m_nDeviceFD,szCmd);
	// Format temperature
	return formatTemp(nTemp);
}

//---------------------------------------------------
/**
  * displayConfig : Display DS1621 configuration
  *
  * @return true if device was initialized
*/
//---------------------------------------------------
bool Ds1621::displayConfig(){
	float fThresholdHigh, fThresholdLow;
	unsigned char szConfig = getConfig();

	// Sanity check
	M_B_DS1621_IS_DEVICE_UP


	printf("\n========================================\n");
	printf("==== CONFIGURATION OF DEVICE AT %X ====\n",m_szAddres);
	printf("========================================\n");
	printf("DONE THF TLF NVB X X POL 1SHOT\n");
	if((szConfig & K_DS1621_DONE_CONFIG) == K_DS1621_DONE_CONFIG){
		printf(" 1");
	}else{
		printf(" 0");
	}
	if((szConfig & K_DS1621_THF_CONFIG) == K_DS1621_THF_CONFIG){
		printf("    1");
	}else{
		printf("    0");
	}
	if((szConfig & K_DS1621_TLF_CONFIG) == K_DS1621_TLF_CONFIG){
		printf("   1");
	}else{
		printf("   0");
	}
	if((szConfig & K_DS1621_NVB_CONFIG) == K_DS1621_NVB_CONFIG){
		printf("   1");
	}else{
		printf("   0");
	}
	if((szConfig & K_DS1621_POL_CONFIG) == K_DS1621_POL_CONFIG){
		printf("       1");
	}else{
		printf("       0");
	}
	if((szConfig & K_DS1621_1SHOT_CONFIG) == K_DS1621_1SHOT_CONFIG){
		printf("    1\n");
	}else{
		printf("    0\n");
	}
	fThresholdHigh = getThresholdTemp(false);
	printf("High temp threshold\t = %2.2f\n",fThresholdHigh);
	fThresholdLow = getThresholdTemp(true);
	printf("Low temp threshold\t = %2.2f\n",fThresholdLow);
	return true;
}

//************* PRIVATE SECTION *************************

//---------------------------------------------------
/**
  * setConfig : set device configuration
  *
  * @param szConfig is the configuration we want to set
  *
*/
//---------------------------------------------------
void Ds1621::setConfig(unsigned char szConfig){
	writeRegister8bitsi2c(K_DS1621_ACCES_CONFIG,szConfig);
}

//---------------------------------------------------
/**
  * getConfig : get device configuration
  *
  * @return the configuration of the device
  *
*/
//---------------------------------------------------
unsigned char Ds1621::getConfig(void){
	unsigned char szConfig;

	// DONE THF TLF NVB X X POL 1SHOT
	// DONE = Conversion Done bit. “1” = Conversion complete, “0” = Conversion in progress.
	// THF = Temperature High Flag. This bit will be set to “1” when the temperature is greater than or
	// equal to the value of TH. It will remain “1” until reset by writing “0” into this location or removing power
	// from the device. This feature provides a method of determining if the DS1621 has ever been subjected to
	// temperatures above TH while power has been applied.
	// TLF = Temperature Low Flag. This bit will be set to “1” when the temperature is less than or equal
	// to the value of TL. It will remain “1” until reset by writing “0” into this location or removing power from
	// the device. This feature provides a method of determining if the DS1621 has ever been subjected to
	// temperatures below TL while power has been applied.
	// NVB = Nonvolatile Memory Busy flag. “1” = Write to an E2 memory cell in progress, “0” = nonvolatile memory is not busy.
	// A copy to E2 may take up to 10 ms.
	// POL = Output Polarity Bit. “1” = active high, “0” = active low. This bit is nonvolatile.
	// 1SHOT = One Shot Mode. If 1SHOT is “1”, the DS1621 will perform one temperature conversion upon
	// receipt of the Start Convert T protocol. If 1SHOT is “0”, the DS1621 will continuously perform
	// temperature conversions. This bit is nonvolatile.

	szConfig = (unsigned char)wiringPiI2CReadReg8(m_nDeviceFD,K_DS1621_ACCES_CONFIG);
	return szConfig;
}

//---------------------------------------------------
/**
  * waitEndOfConversion : Test configuration and return when conversion is done
  *
*/
//---------------------------------------------------
void Ds1621::waitEndOfConversion(void){
	unsigned char szConfig;
	// Wait end of conversion
	do {
	   // Get Config register and test bit DONE
	   szConfig = getConfig();
	   usleep(400);
	}while((szConfig & K_DS1621_DONE_CONFIG) != K_DS1621_DONE_CONFIG);
}

//---------------------------------------------------
/**
  * startStopConvert : Start or stop the temperature conversion
  *
  * @param isStop if true, stop the conversion, if false start it
  *
*/
//---------------------------------------------------
void Ds1621::startStopConvert(bool isStop){
	unsigned char szCmd = K_DS1621_START_CONVERT;
	if(true == isStop){
		szCmd = K_DS1621_STOP_CONVERT;
	}
	writei2c(szCmd);
}

//---------------------------------------------------
/**
  * formatTemp : format the converted temperature
  *
  * @param nTemp is the converted value
  *
  * @return an analog temperature
  *
*/
//---------------------------------------------------
float Ds1621::formatTemp(signed int nTemp){
	float fTemp;
	// Format temperature
	// Negative ?
	if((signed char) nTemp < 0){
		// LSB is the temperature
		// Bit 7 of MSB is set in case of  -0.5C
		fTemp = (signed char)nTemp - 0.5 * (nTemp >> 15);
	}else {
		// LSB is the temperature
		// Bit 7 of MSB is set in case of  +0.5C
		fTemp = (unsigned char)nTemp + 0.5 * (nTemp >> 15);
	}
	return fTemp;
}

//---------------------------------------------------
/**
  * writei2c : write at low level
  *
  * @param szData is the data to write
  *
*/
//---------------------------------------------------
void Ds1621::writei2c(unsigned char szData){
	int nRes;
	nRes = wiringPiI2CWrite(m_nDeviceFD,szData);
	if(0 != nRes){
		throw std::runtime_error("[Error] i2c write error");
	}
}

//---------------------------------------------------
/**
  * writeRegister8bitsi2c : write 8 bits data into a register at low level
  *
  * @param szRegister is the register to write
  * @param szData is the data to write
  *
*/
//---------------------------------------------------
void Ds1621::writeRegister8bitsi2c(unsigned char szRegister, unsigned char szData){
	int nRes;
	nRes = wiringPiI2CWriteReg8(m_nDeviceFD,szRegister,szData);
	if(0 != nRes){
		throw std::runtime_error("[Error] i2c 8 bits register write error");
	}
}

//---------------------------------------------------
/**
  * writeRegister16bitsi2c : write 16 bits data into a register at low level
  *
  * @param szRegister is the register to write
  * @param nData is the data to write
  *
*/
//---------------------------------------------------
void Ds1621::writeRegister16bitsi2c(unsigned char szRegister, int nData){
	int nRes;
	nRes = wiringPiI2CWriteReg16(m_nDeviceFD,szRegister,nData);
	if(0 != nRes){
		throw std::runtime_error("[Error] i2c 16 bits register write error");
	}
}

//---------------------------------------------------
/**
  * checkThresholdTemperatureRange : check temperature threshold value
  *
  * @param fTemp is the temperature to check
  *
*/
//---------------------------------------------------
void Ds1621::checkThresholdTemperatureRange(float &fTemp){
	if(fTemp < K_DS1621_MIN_THRESHOLD_TEMP){
		fTemp = K_DS1621_MIN_THRESHOLD_TEMP;
	}
	if(fTemp > K_DS1621_MAX_THRESHOLD_TEMP){
		fTemp = K_DS1621_MAX_THRESHOLD_TEMP;
	}
}
