/*--------------------------------------------------------
 *
 *--------------------------------------------------------
 * Project    : I2C
 * Sub-Project: KS0108Display.cpp
 *
 * This code is distributed under the GNU Public License
 * which can be found at http://www.gnu.org/licenses/gpl.txt
 *
 * Author Patrick DELVENNE and ProcessUX 2018
 * Some parts of driver was written by Sami Varjo and Rados³aw Kwiecieñ
 *--------------------------------------------------------
 */

#include <wiringPiI2C.h>
#include <exception>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "KS0108Display.h"
#include "font5x8.h"
#include "corsiva_12.h"
#include "arial_bold_14.h"

//---------------------------------------------------
/**
  * Constructor
  * @param szDataAddres is the I2C addres of the device for Data Port
  * @param szCmdAddres is the I2C addres of the device for Command Port
  *
  * Interface is done with two PCF8574
*/
//---------------------------------------------------
KS0108Display::KS0108Display(unsigned char szDataAddres, unsigned char szCmdAddres){
    m_szDataAddres          = szDataAddres;
    m_szCmdAddres           = szCmdAddres;
    m_szCmd                 = 0;
    m_szData                = 0;
    m_szPosX                = 0;
    m_szPosY                = 0;
    m_isDeviceInitialized   = false;
}

//---------------------------------------------------
/**
  * Destructor
*/
//---------------------------------------------------
KS0108Display::~KS0108Display(){
}

//---------------------------------------------------
/**
  * init : Init LCD
  *
*/
//---------------------------------------------------
void
KS0108Display::init(){
    // Setup the device
    unsigned char szI;
    m_nDeviceDataFD = wiringPiI2CSetup (m_szDataAddres);
    m_nDeviceCmdFD = wiringPiI2CSetup (m_szCmdAddres);
    if(( -1 != m_nDeviceDataFD) && (-1 != m_nDeviceCmdFD)){
        try{
            m_szCmd = (K_KS0108_RS_MASK | K_KS0108_RW_MASK | K_KS0108_EN_MASK | K_KS0108_CS1_MASK | K_KS0108_CS2_MASK | K_KS0108_RST_MASK);
            writei2c(m_nDeviceCmdFD,m_szCmd);
            usleep(10);
            // Remove RST
            m_szCmd &= ~K_KS0108_RST_MASK;
            writei2c(m_nDeviceCmdFD,m_szCmd);
            usleep(10);
            m_szCmd |= K_KS0108_RST_MASK;
            writei2c(m_nDeviceCmdFD,m_szCmd);
            // Controls the display ON for all controllers
            // D7 D6 D5 D4 D3 D2 D1 D0
            //  0  0  1  1  1  1  1 0/1
            for(szI = 0; szI < K_KS0108_NB_CTRL; szI++){
                writeCommand((K_KS0108_DISPLAY_ON_CMD | K_KS0108_ON), szI);
            }

            m_isDeviceInitialized = true;
        }catch(std::exception const& e){
            printf("[KS0108Display] %s\n",e.what());
        }
    }
}

//---------------------------------------------------
/**
  * cls : clear the screen
  *
*/
//---------------------------------------------------
void
KS0108Display::cls(){

    // Sanity check
    M_KS0108_IS_DEVICE_UP

    unsigned char szPages;
    unsigned char szPixel;

    // Clear LCD
    // Browse all pages
    for(szPages = 0; szPages < K_KS0108_PAGES_PER_CTRL; szPages++){
        // Set page address
        setAddress(0,szPages);
        // Clear all lines of this page of display memory
        for(szPixel = 0; szPixel< K_KS0108_SCREEN_WIDTH; szPixel++){
            writeData(0x00);
        }
    }
    setHome();
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
void
KS0108Display::displayStringAtPosition(const char* pData, unsigned char szLine, unsigned char szCol){

    // Sanity check
    M_KS0108_IS_DEVICE_UP

    setCursorAtPosition(szLine,szCol);
    displayString(pData);
}

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
void
KS0108Display::displayStringWithFontAtPosition(const char* pData, unsigned char szFont, unsigned char szLine, unsigned char szCol){

    // Sanity check
    M_KS0108_IS_DEVICE_UP

    setCursorAtPosition(szLine,szCol);
    switch(szFont){
        case 0:
            displayString(pData,Corsiva_12);
            break;

        case 1:
            displayString(pData,Arial_Bold_14);
            break;

        default:
            displayString(pData);
            break;
    }
}

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
void
KS0108Display::drawRect(unsigned char szX, unsigned char szY, unsigned char szL, unsigned char szW){

    // Sanity check
    M_KS0108_IS_DEVICE_UP

    unsigned char szIndex;

    // First draw vertical lines of both sides
    for (szIndex = 0; szIndex < szW; szIndex++) {
        setPixel(szX, szY + szIndex);
        setPixel(szX + szL - 1, szY + szIndex);
    }
    // First szIndex horizontal lines of both sides
    for (szIndex = 0; szIndex < szL; szIndex++) {
        setPixel(szX + szIndex, szY);
        setPixel(szX + szIndex, szY + szW - 1);
    }
}

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
void
KS0108Display::drawLine(unsigned char szXo, unsigned char szYo, unsigned char szXd, unsigned char szYd){

    // Sanity check
    M_KS0108_IS_DEVICE_UP

    int nTwoDxAccumulatedError, nTwoDyAccumulatedError;

    // Horizontal position
    int nDx = szXd-szXo;
    // Vertical position
    int nDy = szYd-szYo;

    int nTwoDx = nDx * 2;
    int nTwoDy = nDy * 2;

    int nCurrentX = szXo;
    int nCurrentY = szYo;

    // Assume forward drawing
    int nXinc = 1;
    int nYinc = 1;

    // Horizontal delta is negative
    if(nDx < 0) {
        // We will be moving backward
        nXinc = -1;
        // Keep delta positive
        nDx = -nDx;
        nTwoDx = -nTwoDx;
    }

    // Vertical delta is negative
    if (nDy < 0) {
        nYinc = -1;
        nDy = -nDy;
        nTwoDy = -nTwoDy;
    }

    // Set first pixel of the line
    setPixel(szXo,szYo);

    // Sanity check. Is the line not a pixel
    if ((nDx != 0) || (nDy != 0)){
        // Vertical delta is less than horizontal delta
        if (nDy <= nDx){
            nTwoDxAccumulatedError = 0;
            do {
                // Current position is changed
                nCurrentX += nXinc;
                nTwoDxAccumulatedError += nTwoDy;
                // Time to change Y position
                if(nTwoDxAccumulatedError > nDx){
                    nCurrentY += nYinc;
                    nTwoDxAccumulatedError -= nTwoDx;
                }
                setPixel(nCurrentX,nCurrentY);
            }while (nCurrentX != szXd);
        }else{
            nTwoDyAccumulatedError = 0;
            do{
                nCurrentY += nYinc;
                nTwoDyAccumulatedError += nTwoDx;
                if(nTwoDyAccumulatedError > nDy){
                    nCurrentX += nXinc;
                    nTwoDyAccumulatedError -= nTwoDy;
                }
                setPixel(nCurrentX,nCurrentY);
            }while (nCurrentY != szYd);
        }
    }
}

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
void
KS0108Display::drawBitmap(char *pData, unsigned char szX, unsigned char szY, unsigned char szDx, unsigned char szDy){
    unsigned char szI, szJ;
    unsigned int nIndex;

    // Sanity check
    M_KS0108_IS_DEVICE_UP

    nIndex = 0;
    for(szJ = 0; szJ < szDy / K_KS0108_PAGES_PER_CTRL; szJ++){
        setAddress(szX,szY + szJ);
        for(szI = 0; szI < szDx; szI++){
            writeData(pData[nIndex++]);
        }
    }
}

//---------------------------------------------------
/**
  * setStartLine : Indicate the display data RAM displayed at the top of the screen
  *
  * @param szStart is the position in the line from 0 to 63
  * @note All controllers are affected by this operation
  *
*/
//---------------------------------------------------
void
KS0108Display::setStartLine(unsigned char szStart){
    unsigned char szCtrl;

    if(szStart < K_KS0108_X_PIXELS_PER_CTRL){
        // Browse all controllers
        for(szCtrl = 0; szCtrl < K_KS0108_NB_CTRL; szCtrl++){
            // Indicate the display data RAM displayed at the top of the screen
            // D7 D6 D5 D4 D3 D2 D1 D0
            //  1  1  Y  Y  Y  Y  Y  Y
            writeCommand(K_KS0108_DISPLAY_START_LINE | szStart , szCtrl);
        }
    }else{
        throw std::invalid_argument("[Error] setStartLine start out of range [0-63]");
    }
}

//************* PRIVATE SECTION *************************

//---------------------------------------------------
/**
  * drawChar : draw a char with the given font
  *
  * @param szC is the char to display
  * @param pFont is the font to use
  *
*/
//---------------------------------------------------
void
KS0108Display::drawChar(unsigned char szC, const unsigned char* pFont){

    unsigned char szI,szJ,szPage,szData;
    unsigned char szX0      = m_szPosX;
    unsigned char szY0      = m_szPosY;
    unsigned char szWidth   = 0;
    unsigned int nIndex     = 0;

    // See font definition .h file for data struct
    unsigned char szHeight      = pFont[3];
    unsigned char szBytes       = (szHeight+7)/8;
    unsigned char szFirstChar   = pFont[4];
    unsigned char szCharCount   = pFont[5];

    if ((szC < szFirstChar) || (szC > szFirstChar + szCharCount)){
         return;
    }

    szC -= szFirstChar;

    for(szI = 0; szI < szC; szI++){
        nIndex += pFont[6+szI]; // index 6 is the start of font width table
    }

    nIndex = nIndex * szBytes + szCharCount + 6;
    szWidth = pFont[6 + szC];

    for(szI = 0;szI < szBytes; szI++){
        szPage = szI * szWidth;
        for(szJ = 0; szJ < szWidth; szJ++){
            szData = pFont[nIndex + szPage + szJ];
            if(szHeight < (szI + 1) * 8){
                szData >>= (szI+1) *8 - szHeight;
            }
            writeData(szData);
        }
        writeData(0x00);
        // If the font is higher that 8 pixels
        setAddress(m_szPosX-szWidth-1,m_szPosY+1);
    }
    // Next position for next char
    setAddress(szX0 + szWidth + 1,szY0);
}

//---------------------------------------------------
/**
  * setCursorAtPosition : set TEXT pointer to the given coordonate
  *
  * @param szLine is line number
  * @param szCol is col number
  *
*/
//---------------------------------------------------
void
KS0108Display::setCursorAtPosition(unsigned char szLine, unsigned char szCol){

    if((szLine < K_KS0108_PAGES_PER_CTRL) && (szCol < K_KS0108_MAX_TXT_COL)){
        setAddress(szCol * K_KS0108_X_PIXEL_PER_CHAR, szLine);
    }else{
        throw std::invalid_argument("[Error] setCursorAtPosition arg out of range");
    }
}

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
void
KS0108Display::setPixel(unsigned char szX, unsigned char szY, bool isVisible){
    unsigned char szTmp;
    setAddress(szX, (szY / K_KS0108_PAGES_PER_CTRL));
    // Read actual data
    szTmp = readData();
    setAddress(szX, (szY / K_KS0108_PAGES_PER_CTRL));
    szTmp = readData();
    setAddress(szX, (szY / K_KS0108_PAGES_PER_CTRL));
    szTmp |= (1 << (szY % K_KS0108_PAGES_PER_CTRL));
    writeData(szTmp);
}

//---------------------------------------------------
/**
  * writeChar : display a single char by building a font
  *
  * @param szChar is the char to display
  *
*/
//---------------------------------------------------
void
KS0108Display::writeChar(unsigned char szChar){
    unsigned char szIndex;
    szChar -= 0x20;
    for(szIndex = 0; szIndex < K_KS0108_SCREEN_FONT_WIDTH; szIndex++){
        writeData(font5x8[(K_KS0108_SCREEN_FONT_WIDTH * szChar) + szIndex]);
    }
    writeData(0x00);
}
//---------------------------------------------------
/**
  * displayString : display a string at the current cursor position
  *
  * @param pData is the pointer at the data to write
  * @param pFont is the pointer at the font to use
  * @note if the font pointer is NULL, display is done with the default font
  *
*/
//---------------------------------------------------
void
KS0108Display::displayString(const char* pData, const unsigned char *pFont){
    if(NULL != pData){
        unsigned char szIndex = 0;
        //while((0 != pData[szIndex]) && (szIndex < K_KS0108_MAX_TXT_COL)){
        while(0 != pData[szIndex]){
            if(NULL == pFont){
                writeChar(pData[szIndex]);
            }else{
                drawChar(pData[szIndex],pFont);
            }
            szIndex++;
        };
    }else{
        throw std::invalid_argument("[Error] displayString NULL data");
    }
}

//---------------------------------------------------
/**
  * enableController :
  *
  * @param szCtrl is the controller to select
  *
*/
//---------------------------------------------------
void
KS0108Display::enableController(unsigned char szCtrl){

    switch(szCtrl){
        case 1:
            m_szCmd &= ~K_KS0108_CS1_MASK;
            m_szCmd |= K_KS0108_CS2_MASK;
            break;

        case 0:
            m_szCmd &= ~K_KS0108_CS2_MASK;
            m_szCmd |= K_KS0108_CS1_MASK;
            break;
    }
    writei2c(m_nDeviceCmdFD,m_szCmd);
}

//---------------------------------------------------
/**
  * setHome : Initialize all addresses
  *
*/
//---------------------------------------------------
void
KS0108Display::setHome(){
    // Initialize addresses/positions
    setStartLine(0);
    setAddress(0,0);
    m_szPosX = 0;
    m_szPosY = 0;
}

//---------------------------------------------------
/**
  * setYAddress : Set the Y address in the Y address counter
  *
  * @param szX is the position in the line from 0 to 127
  * @note automatically select the correct controller
  *
*/
//---------------------------------------------------
void
KS0108Display::setYAddress(unsigned char szX){
    unsigned char szCtrl;
    if(szX < K_KS0108_SCREEN_WIDTH){
        m_szPosX = szX;
        // Browse all controllers
        // Clear all Y addresses for all controllers
        for(szCtrl = 0; szCtrl < K_KS0108_NB_CTRL; szCtrl++){
            // Set the Y address in the Y address counter
            // D7 D6 D5 D4 D3 D2 D1 D0
            //  0  1  Y  Y  Y  Y  Y  Y
            writeCommand(K_KS0108_DISPLAY_SET_Y | 0, szCtrl);
        }
        // Finally set the Y address for the selected controller
        writeCommand(K_KS0108_DISPLAY_SET_Y | (szX % K_KS0108_X_PIXELS_PER_CTRL), (szX / K_KS0108_X_PIXELS_PER_CTRL));
    }else{
        throw std::invalid_argument("[Error] setYAddress counter out of range [0-127]");
    }
}

//---------------------------------------------------
/**
  * setPageRegister : Set the page in the X address counter
  *
  * @param szY is the page from 0 to 7
  *
*/
//---------------------------------------------------
void
KS0108Display::setPageRegister(unsigned char szY){
    unsigned char szCtrl;
    if(szY < K_KS0108_PAGES_PER_CTRL){
        m_szPosY = szY;
        // Browse all controllers
        for(szCtrl = 0; szCtrl < K_KS0108_NB_CTRL; szCtrl++){
            // Set page address for all controllers
            // D7 D6 D5 D4 D3 D2 D1 D0
            //  1  0  1  1  1  X  X  X
            writeCommand(K_KS0108_DISPLAY_SET_X | szY, szCtrl);
        }
    }else{
        throw std::invalid_argument("[Error] setPageRegister page out of range [0-7]");
    }
}

//---------------------------------------------------
/**
  * setAddress : Set the address inside the plot matrix
  *
  * @param szX is the position in the line from 0 to 127
  * @param szY is page from 0 to 7
  *
*/
//---------------------------------------------------
void
KS0108Display::setAddress(unsigned char szX, unsigned char szY){
    // Set addresses
    setPageRegister(szY);
    setYAddress(szX);
}

//---------------------------------------------------
/**
  * waitBusyFlag : read the busy flag of the LCD
  *
  * @param szCtrl is the controller to select
  *
*/
//---------------------------------------------------
void
KS0108Display::waitBusyFlag(unsigned char szCtrl){
    unsigned char szStatus;
    // Select the controller by setting CSx to L
    enableController(szCtrl);
    // Select the status register read operation
    // RS R/W
    //  L H
    m_szCmd &= ~K_KS0108_RS_MASK;
    m_szCmd |= (K_KS0108_RW_MASK | K_KS0108_EN_MASK);
    writei2c(m_nDeviceCmdFD,m_szCmd);
    usleep(1);
    do{
        // Ready the status register
        // D7    D6 D5      D4   D3 D2 D1 D0
        // BUSY  0  ON/OFF  RST   0  0  0  0
        szStatus = readi2c(m_nDeviceDataFD);
        m_szCmd &= ~K_KS0108_EN_MASK;
        writei2c(m_nDeviceCmdFD,m_szCmd);
        //printf("%d\n",szStatus);
    }while (szStatus &  K_KS0108_DISPLAY_STATUS_BUSY);
    // RS R/W
    //  L   L
    m_szCmd &= ~K_KS0108_RW_MASK;
    writei2c(m_nDeviceCmdFD,m_szCmd);
}

//---------------------------------------------------
/**
  * readData : read a data from lcd at current position
  *
  * @return the data
  *
*/
//---------------------------------------------------
unsigned char
KS0108Display::readData(void){
    unsigned char szData;
    // Each controller can handle 64 dots, so we have to know the one to talk to
    unsigned char szCurrentCtrl = m_szPosX / K_KS0108_X_PIXELS_PER_CTRL;

    // Wait for busy bit to be reset
    waitBusyFlag(szCurrentCtrl);

    // Select read data operation
    // RS R/W
    //  H H
    m_szCmd |= (K_KS0108_RW_MASK | K_KS0108_RS_MASK);
    writei2c(m_nDeviceCmdFD,m_szCmd);

    // EN
    // H
    //    ____
    //___|
    m_szCmd |= K_KS0108_EN_MASK;
    writei2c(m_nDeviceCmdFD,m_szCmd);
    usleep(K_KS0108_STROBE_DELAY);

    // Read the data
    szData = readi2c(m_nDeviceDataFD);

    // Finally latch data on the falling edge of EN
    // EN
    // L
    //    ____
    //        |____
    m_szCmd &= ~K_KS0108_EN_MASK;
    writei2c(m_nDeviceCmdFD,m_szCmd);

    // Increment the pointer
    m_szPosX++;
    if(m_szPosX >= K_KS0108_SCREEN_WIDTH){
        m_szPosY++;
        if(m_szPosY >= K_KS0108_PAGES_PER_CTRL){
            m_szPosY = 0;
        }
        setPageRegister(m_szPosY);
        setYAddress(0);
    }
    return szData;
}

//---------------------------------------------------
/**
  * writeCommand : write a 8 bits command to lcd
  *
  * @param nData is the 8 bit data to write
  * @param szCtrl is the controller to select
  *
*/
//---------------------------------------------------
void
KS0108Display::writeCommand(unsigned char szData, unsigned char szCtrl){
    // Wait for busy bit to be reset
    waitBusyFlag(szCtrl);

    // Select write command operation
    // RS R/W
    //  L   L
    m_szCmd &= ~(K_KS0108_RW_MASK | K_KS0108_RS_MASK);
    writei2c(m_nDeviceCmdFD,m_szCmd);

    // Write the command on the bus
    writei2c(m_nDeviceDataFD,szData);
    //    ____
    //___|    |____
    strobe();
}

//---------------------------------------------------
/**
  * writeData : write a data to lcd
  *
  * @param szData is the data to write
  *
*/
//---------------------------------------------------
void
KS0108Display::writeData(unsigned char szData){
    // Each controller can handle 64 dots, so we have to know the one to talk to
    unsigned char szCurrentCtrl = m_szPosX / K_KS0108_X_PIXELS_PER_CTRL;

    // Wait for busy bit to be reset
    waitBusyFlag(szCurrentCtrl);

    // Select write data operation
    // RS R/W
    //  H   L
    m_szCmd |= K_KS0108_RS_MASK;
    //writei2c(m_nDeviceCmdFD,m_szCmd);
    m_szCmd &= ~K_KS0108_RW_MASK;
    writei2c(m_nDeviceCmdFD,m_szCmd);

    // Write the display data on the bus
    writei2c(m_nDeviceDataFD,szData);
    //    ____
    //___|    |____
    strobe();
    // After writing operation address is increased by 1 automatically
    // Increment the pointer
    m_szPosX++;
    if(m_szPosX >= K_KS0108_SCREEN_WIDTH){
        m_szPosY++;
        if(m_szPosY >= K_KS0108_PAGES_PER_CTRL){
            m_szPosY = 0;
        }
        setPageRegister(m_szPosY);
        // Reset colum position to 0
        setYAddress(0);
    }
}

//---------------------------------------------------
/**
  * stobe : clocks EN to latch command or/and data
  *
  *
*/
//---------------------------------------------------
void
KS0108Display::strobe(){
    // EN
    // H
    m_szCmd |= K_KS0108_EN_MASK;
    writei2c(m_nDeviceCmdFD,m_szCmd);
    usleep(K_KS0108_STROBE_DELAY);
    // Finally latch data on the falling edge of EN
    // EN
    // L
    m_szCmd &= ~K_KS0108_EN_MASK;
    writei2c(m_nDeviceCmdFD,m_szCmd);
    usleep(K_KS0108_STROBE_DELAY);
}

//---------------------------------------------------
/**
  * writei2c : write at low level
  *
  * @param nDeviceID is the deviceIdentifier
  * @param szData is the data to write
  *
*/
//---------------------------------------------------
void
KS0108Display::writei2c(int nDeviceFD, unsigned char szData){
    int nRes;
    nRes = wiringPiI2CWrite(nDeviceFD,szData);
    if(0 != nRes){
        throw std::runtime_error("[Error] i2c write error");
    }
}

//---------------------------------------------------
/**
  * readi2c : read at low level
  *
  * @param nDeviceID is the deviceIdentifier
  * @return the read data
  *
*/
//---------------------------------------------------
unsigned char
KS0108Display::readi2c(int nDeviceFD){
    int nRes;
    // The master needs to write 1 to the register to set the port as an input mode
    writei2c(nDeviceFD,0xFF);
    nRes = wiringPiI2CRead(nDeviceFD);
    if(nRes < 0){
        throw std::runtime_error("[Error] i2c read error");
    }
    return (unsigned char)nRes;
}

