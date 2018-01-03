/*--------------------------------------------------------
 *
 *--------------------------------------------------------
 * Project    : I2C
 * Sub-Project: KS0108Display.h
 *
 * This code is distributed under the GNU Public License
 * which can be found at http://www.gnu.org/licenses/gpl.txt
 *
 * Author Patrick DELVENNE and ProcessUX 2018
 * Some parts of driver was written by Sami Varjo and Rados³aw Kwiecieñ
 *--------------------------------------------------------
 */

#pragma once

// First PCF8574 is the DATA Port
// P7 P6 P5 P4 P3 P2 P1 P0
// D7 D6 D5 D4 D3 D2 D1 D0

// Second PCF8574 is the CMD port
// P7 P6 P5  P4  P3   P2 P1 P0
//       RST CS2 CS1  E  RW RS

// LCD Registers
const unsigned char K_KS0108_RS_MASK                = 0x01;
const unsigned char K_KS0108_RW_MASK                = 0x02;
const unsigned char K_KS0108_EN_MASK                = 0x04;
const unsigned char K_KS0108_CS1_MASK               = 0x08;
const unsigned char K_KS0108_CS2_MASK               = 0x10;
const unsigned char K_KS0108_RST_MASK               = 0x20;

const unsigned char K_KS0108_SCREEN_WIDTH           = 128;
const unsigned char K_KS0108_SCREEN_HEIGHT          = 64;
const unsigned char K_KS0108_SCREEN_FONT_WIDTH      = 5;
const unsigned char K_KS0108_X_PIXEL_PER_CHAR       = 6;

const unsigned char K_KS0108_X_PIXELS_PER_CTRL      = 64;
const unsigned char K_KS0108_PAGES_PER_CTRL         = 8;
const unsigned char K_KS0108_NB_CTRL                = (K_KS0108_SCREEN_WIDTH / K_KS0108_X_PIXELS_PER_CTRL);
const unsigned char K_KS0108_MAX_TXT_COL            = (K_KS0108_SCREEN_WIDTH / K_KS0108_SCREEN_FONT_WIDTH);


const unsigned char K_KS0108_DISPLAY_SET_Y          = 0x40;
const unsigned char K_KS0108_DISPLAY_SET_X          = 0xB8;
const unsigned char K_KS0108_DISPLAY_START_LINE     = 0xC0;
const unsigned char K_KS0108_DISPLAY_ON_CMD         = 0x3E;
const unsigned char K_KS0108_ON                     = 0x01;
const unsigned char K_KS0108_OFF                    = 0x00;
const unsigned char K_KS0108_DISPLAY_STATUS_BUSY    = 0x80;

const unsigned char K_KS0108_STROBE_DELAY           = 0x01;

#define M_KS0108_IS_DEVICE_UP                       if(false == m_isDeviceInitialized) return;

class KS0108Display{
    public:
        //---------------------------------------------------
        /**
          * Constructor
          * @param szDataAddres is the I2C addres of the device for Data Port
          * @param szCmdAddres is the I2C addres of the device for Command Port
          *
          * Interface is done with two PCF8574
        */
        //---------------------------------------------------
        KS0108Display(unsigned char szDataAddres, unsigned char szCmdAddres);

        //---------------------------------------------------
        /**
          * Destructor
        */
        //---------------------------------------------------
        virtual ~KS0108Display();

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
          * cls : clear the screen
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
        void displayStringAtPosition(const char* pData, unsigned char szLine, unsigned char szCol = 0);

        //---------------------------------------------------
        /**
          * displayStringWithFontAtPosition : display a string at the given position with the given font
          *
          * @param pData is the pointer the data to write
          * @param szFont is the font number (0=Corsiva12, 1=Arial Bold)
          * @param szLine is the line number
          * @param szCol is the colum number
          *
        */
        //---------------------------------------------------
        void displayStringWithFontAtPosition(const char* pData, unsigned char szFont, unsigned char szLine, unsigned char szCol);

        //---------------------------------------------------
        /**
          * drawRect : draw a rectangle
          *
          * @param szX is the X coord
          * @param szY is the Y coord
          * @param szL is the lenght
          * @param szW is the width
          *
        */
        //---------------------------------------------------
        void drawRect(unsigned char szX, unsigned char szY, unsigned char szL, unsigned char szW);

        //---------------------------------------------------
        /**
          * drawLine : draw a line
          *
          * @param szXo is the X coord of the origin
          * @param szYo is the Y coord of the origin
          * @param szXd is the X coord of the destination
          * @param szYd is the Y coord of the destination
          *
        */
        //---------------------------------------------------
        void drawLine(unsigned char szXo, unsigned char szYo, unsigned char szXd, unsigned char szYd);

        //---------------------------------------------------
        /**
          * drawBitmap : draw a bitmap
          *
          * @param szX is the X coord of the origin
          * @param szY is the Y coord of the origin
          * @param szDx is the width
          * @param szDy is the height
          *
        */
        //---------------------------------------------------
        void drawBitmap(char *pData, unsigned char szX, unsigned char szY, unsigned char szDx, unsigned char szDy);


    private:
        // Flag to know if operations are valid or not
        bool m_isDeviceInitialized;

        // I2C Addres of the device for Data Port
        unsigned char m_szDataAddres;

        // I2C Addres of the device for Command Port
        unsigned char m_szCmdAddres;

        // File descriptor of the PCF8574 devices
        int m_nDeviceDataFD;
        int m_nDeviceCmdFD;

        // Current data
        unsigned char m_szData;
        // Current command
        unsigned char m_szCmd;

        // Current x position
        unsigned char m_szPosX;
        // Current y position
        unsigned char m_szPosY;

        //---------------------------------------------------
        /**
          * drawchar : draw a char with the given font
          *
          * @param szC is the char to display
          * @param pFont is the font to use
          *
        */
        //---------------------------------------------------
        void drawChar(unsigned char szC, const unsigned char* pFont);


        //---------------------------------------------------
        /**
          * setCursorAtPosition : set pointer to the given coordonate
          *
          * @param szLine is line number
          * @param szCol is col number
          *
        */
        //---------------------------------------------------
        void setCursorAtPosition(unsigned char szX, unsigned char szY);

        //---------------------------------------------------
        /**
          * setPixel : set /unset pixel on screen
          *
          * @param szX is the X coord
          * @param szY is the Y coord
          * @param isVisible if true, set the pixel, if false clear it
          *
        */
        //---------------------------------------------------
        void setPixel(unsigned char szX, unsigned char szY, bool isVisible = true);

        //---------------------------------------------------
        /**
          * writeChar : display a single char by building a font
          *
          * @param szChar is the char to display
          *
        */
        //---------------------------------------------------
        void writeChar(unsigned char szChar);

        //---------------------------------------------------
        /**
          * displayString : display a string at the current cursor position
          *
          * @param pData is the pointer the data to write
          * @param pFont is the pointer at the font to use
          * @note if the font pointer is NULL, display is done with the default font
          *
         */
        //---------------------------------------------------
        void displayString(const char* pData, const unsigned char* pFont=NULL);

        //---------------------------------------------------
        /**
          * enableController :
          *
          * @param szCtrl is the controller to select
          *
        */
        //---------------------------------------------------
        void enableController(unsigned char szCtrl);

        //---------------------------------------------------
        /**
          * setHome : Initialize all addresses
          *
        */
        //---------------------------------------------------
        void setHome();

        //---------------------------------------------------
        /**
          * setYAddress : Set the Y address in the Y address counter
          *
          * @param szX is the position in the line from 0 to 127
          * @note automatically select the correct controller
          *
        */
        //---------------------------------------------------
        void setYAddress(unsigned char szX);

        //---------------------------------------------------
        /**
          * setPageRegister : Set the page in the X address counter
          *
          * @param szY is the page from 0 to 7
          *
        */
        //---------------------------------------------------
        void setPageRegister(unsigned char szY);

        //---------------------------------------------------
        /**
          * setStartLine : Indicate the display data RAM displayed at the top of the screen
          *
          * @param szStart is the position in the line from 0 to 63
          * @note All controllers are affected by this operation
          *
        */
        //---------------------------------------------------
        void setStartLine(unsigned char szStart);

        //---------------------------------------------------
        /**
          * setAddress : Set the address inside the plot matrix
          *
          * @param szX is the position in the line from 0 to 127
          * @param szY is page from 0 to 7
          *
        */
        //---------------------------------------------------
        void setAddress(unsigned char szX, unsigned char szY);

        //---------------------------------------------------
        /**
          * waitBusyFlag : read the busy flag of the LCD
          *
          * @param szCtrl is the controller to select
          *
        */
        //---------------------------------------------------
        void waitBusyFlag(unsigned char szCtrl);

        //---------------------------------------------------
        /**
          * readData : read a data from lcd at current position
          *
          * @return the data
          *
        */
        //---------------------------------------------------
        unsigned char readData(void);

        //---------------------------------------------------
        /**
          * writeCommand : write a 8 bits command to lcd
          *
          * @param szData is the 8 bit data to write
          * @param szCtrl is the controller to select
          *
        */
        //---------------------------------------------------
        void writeCommand(unsigned char szData, unsigned char szCtrl);

        //---------------------------------------------------
        /**
          * writeData : write a data to lcd
          *
          * @param szData is the data to write
          *
        */
        //---------------------------------------------------
        void writeData(unsigned char szData);

        //---------------------------------------------------
        /**
          * stobe : clocks EN to latch command or/and data
          *
          *
        */
        //---------------------------------------------------
        void strobe();

        //---------------------------------------------------
        /**
          * writei2c : write at low level
          *
          * @param nDeviceID is the deviceIdentifier
          * @param szData is the data to write
          *
        */
        //---------------------------------------------------
        void writei2c(int nDeviceID, unsigned char szData);

        //---------------------------------------------------
        /**
          * readi2c : read at low level
          *
          * @param nDeviceID is the deviceIdentifier
          * @return the read data
          *
        */
        //---------------------------------------------------
        unsigned char readi2c(int nDeviceFD);


};
