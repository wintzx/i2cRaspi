/*--------------------------------------------------------
 *
 *--------------------------------------------------------
 * Project    : I2C
 * Sub-Project: i2cTest.cpp
 *
 * This code is distributed under the GNU Public License
 * which can be found at http://www.gnu.org/licenses/gpl.txt
 *
 * Author Patrick DELVENNE and ProcessUX 2018
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

#include <wiringPi.h>
#include <wiringPiI2C.h>
#include "LcdDisplay/LcdDisplay.h"
#include "T6963Display/T6963Display.h"
#include "KS0108Display/KS0108Display.h"
#include "Ds1621/Ds1621.h"
#include "KS0108Display/wintzx.h"

// Devices
const unsigned char K_I2C_T6963_DATA_ADDRES     = 0x20;
const unsigned char K_I2C_T6963_CMD_ADDRES      = 0x21;


//---------------------------------------------------
int main (int argc, char *argv[]){
//---------------------------------------------------

    wiringPiSetup();
    KS0108Display *pDis = new KS0108Display(K_I2C_T6963_DATA_ADDRES,K_I2C_T6963_CMD_ADDRES);
    pDis->init();
    pDis->cls();
    pDis->drawBitmap((char*)ours,0,0,128,64);
    pDis->drawRect(0,0,127,63);
    pDis->drawLine(0,0,127,63);
    pDis->displayStringWithFontAtPosition("http://www.wintzx.fr",0,0,0);
    delete(pDis);
}
