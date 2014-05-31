/*--------------------------------------------------------
 *
 *--------------------------------------------------------
 * Project    : I2C
 * Sub-Project: LcdDisplay.cpp
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
#include "LcdDisplay.h"

//---------------------------------------------------
/**
  * Constructor
  * @param szAddres is the I2C addres of the device
  *
  * In this mode, only LCD pins D4 - D7 are used, D0 - D3 are grounded.
  * Interface is done with a PCF8574
*/
//---------------------------------------------------
LcdDisplay::LcdDisplay(unsigned char szAddres){
	m_szAddres = szAddres;
	m_isDeviceInitialized = false;
}

//---------------------------------------------------
/**
  * Destructor
*/
//---------------------------------------------------
LcdDisplay::~LcdDisplay(){
}

//---------------------------------------------------
/**
  * init : Init LCD
  *
*/
//---------------------------------------------------
void LcdDisplay::init(){
	// Setup the device
	m_nDeviceFD = wiringPiI2CSetup (m_szAddres) ;
	if( -1 != m_nDeviceFD){
		try{
			// Init pattern
			// Function set 0011XXXX,
			write(0x03);
			// Wait 4,1ms
			usleep(4100);
			// Function set 0011XXXX,
			write(0x03);
			// Wait 100µs
			usleep(100);
			// Function set 0011XXXX,
			write(0x03);

			// Activate 4 bits mode
			write(0x02);

			// Function SET
			// 0    0   1   DL  N   F   -   -
			// DL   = 1 → 8 bits Interface
			// DL   = 0 → 4 bits Interface
			// N    = 1 → 2 display lines
			// N    = 0 → 1 display line
			// F    = 1 → 5 x 11 dots matrix
			// F    = 0 → 5 x 8 dots matrix
			// Here we set 4 bits mode, 5x8 matrix and 2 lines display
			write(K_LCD_FUNCTIONSET | K_LCD_2LINE | K_LCD_5x8DOTS | K_LCD_4BITMODE);

			// Display on/off control
			// 0    0   0   0   1   D   C   B
			// D = 1 → Enable display
			// D = 0 → Disable display
			// C = 1 → Enable cursor
			// C = 0 → Disable cursor
			// B = 1 → Blinking cursor
			// B = 0 → Fixed cursor
			// Here we just enable display
			write(K_LCD_DISPLAYCONTROL | K_LCD_DISPLAYON);

			// Clear display
			// 0    0   0   0   0   0   0   1
			write(K_LCD_CLEARDISPLAY);

			// Entry set mode
			// 0    0   0   0   0   1   I/D     S
			// I/D  = 1 → Adress counter incrementation (move cursor one position right)
			// I/D  = 0 → Adress counter decrementation (move cursor one position left)
			// S    = 1 → Shift display to the cursor direction
			// S    = 0 → Display is not shifted
			// Here we set the cursor to be moved on the right with no display shifting
			write(K_LCD_ENTRYMODESET | K_LCD_ENTRYRIGHT);

			usleep(2000);
			m_isDeviceInitialized = true;
		}catch(std::exception const& e){
			printf("[LCD] %s\n",e.what());
		}
	}
}

//---------------------------------------------------
/**
  * cls : clear lcd and set cursor to home
  *
*/
//---------------------------------------------------
void LcdDisplay::cls(){

	// Sanity check
	M_LCD_IS_DEVICE_UP

	write(K_LCD_CLEARDISPLAY);
	write(K_LCD_RETURNHOME);
}

//---------------------------------------------------
/**
  * displayStringAtPosition : display a string at the given position
  *
  * @param pData is the pointer the data to write
  * @param szLine is the line number
  * @param szCol is the colum number
  *
*/
//---------------------------------------------------
void LcdDisplay::displayStringAtPosition(const char* pData, char szLine, char szCol){
	
	// Sanity check
	M_LCD_IS_DEVICE_UP
	
	setCursorAtPosition(szLine,szCol,false,false);
	displayString(pData);
}

//---------------------------------------------------
/**
  * setCursorAtPosition : setCursor
  *
  * @param szLine is the line number
  * @param szCol is the colum number
  * @param isVisible if true, enable the cursor
  * @param isBlinking if true, enable cursor blinking
  *
*/
//---------------------------------------------------
void LcdDisplay::setCursorAtPosition(char szLine, char szCol,bool isVisible, bool isBlinking){
	unsigned char cmdArg;

	// Sanity check
	M_LCD_IS_DEVICE_UP
	
	if(true == isVisible){
		cmdArg = K_LCD_CURSORON;
	}else{
		cmdArg = K_LCD_CURSOROFF;
	}
	if(true == isBlinking){
		cmdArg |= K_LCD_BLINKON;
	}else{
		cmdArg |= K_LCD_BLINKOFF;
	}

	// Move cursor home
	write(K_LCD_RETURNHOME);

	// Set the line position
	setLinePosition(szLine);

	// Now move the cursor
	for(unsigned char szIndex= 0; szIndex < K_LCD_MAX_CHAR_PER_LINE; szIndex++){
		if(szIndex == szCol){
			break;
		}
		write(K_LCD_CURSORSHIFT | K_LCD_CURSORMOVE | K_LCD_MOVERIGHT);
	}

	write(K_LCD_DISPLAYCONTROL | K_LCD_DISPLAYON | cmdArg);
}

//************* PRIVATE SECTION *************************

//---------------------------------------------------
/**
  * write : write a command to lcd
  *
  * @param szData is the command/data to write
  * @param szMode is the mode
  *
*/
//---------------------------------------------------
void LcdDisplay::write(char szData, char szMode){
	// Write Hi Nibble
	writeNibble(szMode | (szData & 0xF0));
	// Write Low Nibble
	writeNibble(szMode | ((szData << 4) & 0xF0));
}

//---------------------------------------------------
/**
  * writeNibble : write a nibble to the lcd
  *
  * @param szData is the data nibble to write
  *
*/
//---------------------------------------------------
void LcdDisplay::writeNibble(char szData){
	writei2c(szData | K_LCD_BACKLIGHT);
	strobe(szData);
}

//---------------------------------------------------
/**
  * stobe : clocks EN to latch command
  *
  * @param szData is the data to write
  *
*/
//---------------------------------------------------
void LcdDisplay::strobe(char szData){
	writei2c(szData | K_LCD_EN_MASK | K_LCD_BACKLIGHT);
	usleep(800);
	writei2c(((szData & ~K_LCD_EN_MASK) | K_LCD_BACKLIGHT));
	usleep(400);
}

//---------------------------------------------------
/**
  * setLinePosition : Set the cursor at the given line position
  *
  * @param szLine is the line where to put the cursor
  *
*/
//---------------------------------------------------
void LcdDisplay::setLinePosition(char szLine){
	char szCmdLine;

	switch(szLine){
			case 1:
				szCmdLine=0x80;
				break;

			case 2:
				szCmdLine=0xC0;
				break;

			case 3:
				szCmdLine=0x94;
				break;

			case 4:
				szCmdLine=0xD4;
				break;

			default:
				throw std::out_of_range("[Error] line number is out of range");
				break;
	}
	write(szCmdLine);
}

//---------------------------------------------------
/**
  * displayString : display a string at the current cursor position
  *
  * @param pData is the pointer the data to write
  *
*/
//---------------------------------------------------
void LcdDisplay::displayString(const char* pData){
	if(NULL != pData){
		int nIndex = 0;
		while((0 != pData[nIndex]) && (nIndex < K_LCD_MAX_CHAR_PER_LINE)){
			write(pData[nIndex],K_LCD_RS_MASK);
			nIndex++;
		};
	}else{
		throw std::invalid_argument("[Error] NULL data");
	}
}

//---------------------------------------------------
/**
  * writei2c : write at low level
  *
  * @param szData is the data to write
  *
*/
//---------------------------------------------------
void LcdDisplay::writei2c(unsigned char szData){
	int nRes;
	nRes = wiringPiI2CWrite(m_nDeviceFD,szData);
	if(0 != nRes){
		throw std::runtime_error("[Error] i2c write error");
	}
}
