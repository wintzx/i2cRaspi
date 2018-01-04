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
#include "KS0108Display/KS0108Display.h"
#include "Ds1621/Ds1621.h"
#include "KS0108Display/wintzx.h"

// Devices
const unsigned char K_I2C_KS0108_DATA_ADDRES    = 0x20;
const unsigned char K_I2C_KS0108_CMD_ADDRES     = 0x21;
const unsigned char K_I2C_KS0108_MAX_CHAR       = 0xFF;

//---------------------------------------------------
int main (int argc, char *argv[]){
//---------------------------------------------------

    int nRet;
    int szX=-1;
    int szY=-1;
    int szdX=-1;
    int szdY=-1;
    int szFont=-1;
    int szStartLine=-1;
    char szLine[K_I2C_KS0108_MAX_CHAR];
    wiringPiSetup();
    KS0108Display *pDis = new KS0108Display(K_I2C_KS0108_DATA_ADDRES,K_I2C_KS0108_CMD_ADDRES);
    pDis->init();
    szLine[0]=0;
    while ((nRet = getopt (argc, argv, "cx:y:X:Y:s:d:rlu:h")) != -1){
        switch(nRet){
            case 'c':
                pDis->cls();
            break;

            case 'y':
                szX=atoi(optarg);
            break;

            case 'x':
                szY=atoi(optarg);
            break;

            case 'X':
                szdX=atoi(optarg);
            break;

            case 'Y':
                szdY=atoi(optarg);
            break;

            case 's':
                strncpy(szLine,optarg,K_I2C_KS0108_MAX_CHAR-1);
            break;

            case 'u':
                szStartLine = atoi(optarg);
                pDis->setStartLine(szStartLine);
            break;

            case 'd':
                szFont = atoi(optarg);
                if((-1 == szX) || (-1 == szY)){
                    fprintf(stderr,"You must set -x and -y\n");
                    return -1;
                }
                if(0 == strlen(szLine)){
                    fprintf(stderr,"You must set -s\n");
                    return -1;
                }
                pDis->displayStringWithFontAtPosition(szLine,szFont,szX,szY);
            break;

            case 'r':
                if((-1 == szX) || (-1 == szY) || (-1 == szdX) || (-1 == szdY)){
                    fprintf(stderr,"You must set -x, -y -X and -Y\n");
                    return -1;
                }
                pDis->drawRect(szY,szX,szdX,szdY);
            break;

            case 'l':
                if((-1 == szX) || (-1 == szY) || (-1 == szdX) || (-1 == szdY)){
                    fprintf(stderr,"You must set -x, -y -X and -Y\n");
                    return -1;
                }
                pDis->drawLine(szY,szX,szdX,szdY);
            break;

            case 'h':
                // On affiche l'aide et on termine.
                fprintf(stderr, "Usage: i2cTest [options] [function]\n"
                                "Functions:\n"
                                "  -c         Clear screen.\n"
                                "  -u n       Scroll text up to n lines.\n"
                                "  -d n       Display string with selected font.\n"
                                "  -r         Draw rectangle starting at (X,Y) to (X+DX),(Y+DY).\n"
                                "  -l         Draw line starting at (X,Y) to (X+DX),(Y+DY).\n"
                                "Options:\n"
                                "  -y n       Y position.\n"
                                "  -x n       X position.\n"
                                "  -dy n      Y delta position.\n"
                                "  -dx n      X delta position.\n"
                                "  -s xx      String to display.\n"
                       );
                return 0;

        }
    }
    delete(pDis);
}
