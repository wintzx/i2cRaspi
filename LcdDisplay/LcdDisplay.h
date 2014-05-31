/*--------------------------------------------------------
 *
 *--------------------------------------------------------
 * Project    : I2C
 * Sub-Project: LcdDisplay.h
 *
 * Copyright Patrick DELVENNE and ProcessUX 2014
 *--------------------------------------------------------
 */

#pragma once

const unsigned char K_LCD_MAX_CHAR_PER_LINE     = 20;

// Commands
const unsigned char K_LCD_CLEARDISPLAY          = 0x01;
const unsigned char K_LCD_RETURNHOME            = 0x02;
const unsigned char K_LCD_ENTRYMODESET          = 0x04;
const unsigned char K_LCD_DISPLAYCONTROL        = 0x08;
const unsigned char K_LCD_CURSORSHIFT           = 0x10;
const unsigned char K_LCD_FUNCTIONSET           = 0x20;
const unsigned char K_LCD_SETCGRAMADDR          = 0x40;
const unsigned char K_LCD_SETDDRAMADDR          = 0x80;

// Flags for display entry mode
const unsigned char K_LCD_ENTRYLEFT             = 0x00;
const unsigned char K_LCD_ENTRYRIGHT            = 0x02;
const unsigned char K_LCD_ENTRYSHIFTINCREMENT   = 0x01;
const unsigned char K_LCD_ENTRYSHIFTDECREMENT   = 0x00;

// Flags for display on/off control
const unsigned char K_LCD_DISPLAYON             = 0x04;
const unsigned char K_LCD_DISPLAYOFF            = 0x00;
const unsigned char K_LCD_CURSORON              = 0x02;
const unsigned char K_LCD_CURSOROFF             = 0x00;
const unsigned char K_LCD_BLINKON               = 0x01;
const unsigned char K_LCD_BLINKOFF              = 0x00;

// Flags for display/cursor shift
const unsigned char K_LCD_DISPLAYMOVE           = 0x08;
const unsigned char K_LCD_CURSORMOVE            = 0x00;
const unsigned char K_LCD_MOVERIGHT             = 0x04;
const unsigned char K_LCD_MOVELEFT              = 0x00;

// Flags for function set
const unsigned char K_LCD_8BITMODE              = 0x10;
const unsigned char K_LCD_4BITMODE              = 0x00;
const unsigned char K_LCD_2LINE                 = 0x08;
const unsigned char K_LCD_1LINE                 = 0x00;
const unsigned char K_LCD_5x10DOTS              = 0x04;
const unsigned char K_LCD_5x8DOTS               = 0x00;

// Flags for backlight control
const unsigned char K_LCD_BACKLIGHT             = 0x08;
const unsigned char K_LCD_NOBACKLIGHT           = 0x00;

// LCD Registers
// 0b00000100 Enable bit
const unsigned char K_LCD_EN_MASK               = 0x04;
// 0b00000010 Read/Write bit
const unsigned char K_LCD_RW_MASK               = 0x02;
// 0b00000001 Register select bit
const unsigned char K_LCD_RS_MASK               = 0x01;


#define M_LCD_IS_DEVICE_UP						if(false == m_isDeviceInitialized) return;

class LcdDisplay{
	public:
		//---------------------------------------------------
		/**
		  * Constructor
		  * @param szAddres is the I2C addres of the device
		*/
		//---------------------------------------------------
		LcdDisplay(unsigned char szAddres);

		//---------------------------------------------------
		/**
		  * Destructor
		*/
		//---------------------------------------------------
		virtual ~LcdDisplay();

		//---------------------------------------------------
		/**
		  * init : Init LCD
		  *
		*/
		//---------------------------------------------------
		void init();

		//---------------------------------------------------
		/**
		  * isDeviceUp : 
		  *
		  * @return true if device was initialized
		*/
		//---------------------------------------------------
		inline bool isDeviceUp(){ return m_isDeviceInitialized;}

		//---------------------------------------------------
		/**
		  * cls : clear lcd and set cursor to home
		  *
		*/
		//---------------------------------------------------
		void cls();

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
		void displayStringAtPosition(const char* pData, char szLine, char szCol = 0);

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
		void setCursorAtPosition(char szLine, char szCol,bool isVisible, bool isBlinking);

	private:
		// Flag to know if operations are valid or not
		bool m_isDeviceInitialized;

		// I2C Addres of the device
		unsigned char m_szAddres;

		// File descriptor of the device
		int m_nDeviceFD;

		//---------------------------------------------------
		/**
		  * stobe : clocks EN to latch command
		  *
		  * @param szData is the data to write
		  *
		*/
		//---------------------------------------------------
		void strobe(char szData);

		//---------------------------------------------------
		/**
		  * writeNibble : write a nibble to the lcd
		  *
		  * @param szData is the data nibble to write
		  *
		*/
		//---------------------------------------------------
		void writeNibble(char szData);

		//---------------------------------------------------
		/**
		  * write : write a command to lcd
		  *
		  * @param szData is the command/data to write
		  * @param szMode is the mode
		  *
		*/
		//---------------------------------------------------
		void write(char szData, char szMode=0);

		//---------------------------------------------------
		/**
		  * setLinePosition : Set the cursor at the given line position
		  *
		  * @param szLine is the line where to put the cursor
		  *
		*/
		//---------------------------------------------------
		void setLinePosition(char szLine);

		//---------------------------------------------------
		/**
		  * displayString : display a string at the current cursor position
		  *
		  * @param pData is the pointer the data to write
		  *
		 */
		//---------------------------------------------------
		void displayString(const char* pData);

		//---------------------------------------------------
		/**
		  * writei2c : write at low level
		  *
		  * @param szData is the data to write
		  *
		*/
		//---------------------------------------------------
		void writei2c(unsigned char szData);

};
