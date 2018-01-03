/*--------------------------------------------------------
 *
 *--------------------------------------------------------
 * Project    : I2C
 * Sub-Project: Ds1621.h
 *
 * This code is distributed under the GNU Public License
 * which can be found at http://www.gnu.org/licenses/gpl.txt
 *
 * Author Patrick DELVENNE and ProcessUX 2018
 *--------------------------------------------------------
 */

#pragma once

// Commands
const unsigned char K_DS1621_START_CONVERT      = 0xEE;
const unsigned char K_DS1621_STOP_CONVERT       = 0x22;
const unsigned char K_DS1621_READ_TEMP          = 0xAA;
const unsigned char K_DS1621_READ_COUNTER       = 0xA8;
const unsigned char K_DS1621_READ_SLOPE         = 0xA9;
const unsigned char K_DS1621_ACCES_CONFIG       = 0xAC;
const unsigned char K_DS1621_ACCES_TH           = 0xA1;
const unsigned char K_DS1621_ACCES_TL           = 0xA2;

// Config
const unsigned char K_DS1621_DONE_CONFIG        = 0x80;
const unsigned char K_DS1621_THF_CONFIG         = 0x40;
const unsigned char K_DS1621_TLF_CONFIG         = 0x20;
const unsigned char K_DS1621_NVB_CONFIG         = 0x10;
const unsigned char K_DS1621_POL_CONFIG         = 0x02;
const unsigned char K_DS1621_1SHOT_CONFIG       = 0x01;

const float K_DS1621_MIN_THRESHOLD_TEMP         =-55.0;
const float K_DS1621_MAX_THRESHOLD_TEMP         =-125.0;

#define M_F_DS1621_IS_DEVICE_UP                 if(false == m_isDeviceInitialized) return 0.0;
#define M_B_DS1621_IS_DEVICE_UP                 if(false == m_isDeviceInitialized) return false;

class Ds1621{
    public:
        //---------------------------------------------------
        /**
          * Constructor
          * @param szAddres is the I2C addres of the device
        */
        //---------------------------------------------------
        Ds1621(unsigned char szAddres);

        //---------------------------------------------------
        /**
          * Destructor
        */
        //---------------------------------------------------
        virtual ~Ds1621();

        //---------------------------------------------------
        /**
          * init : Init DS1621
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
          * displayConfig : Display DS1621 configuration
          *
          * @return true if device was initialized
        */
        //---------------------------------------------------
        bool displayConfig();

        //---------------------------------------------------
        /**
          * getLRTemp : Get the temperature with a low resolution (0.5C)
          *
          * @return the temperature
        */
        //---------------------------------------------------
        float getLRTemp();

        //---------------------------------------------------
        /**
          * getHRTemp : Get the temperature with a high resolution (0.01C)
          *
          * @return the temperature
        */
        //---------------------------------------------------
        float getHRTemp();

        //---------------------------------------------------
        /**
          * isTHF : read the THF
          *
          * @return true if the DS1621 has ever been subjected to temperatures above TH, false otherwise
          *
        */
        //---------------------------------------------------
        bool isTHF(void);

        //---------------------------------------------------
        /**
          * isTLF : read the TLF
          *
          * @return true if the DS1621 has ever been subjected to temperatures below TL, false otherwise
          *
        */
        //---------------------------------------------------
        bool isTLF(void);

        //---------------------------------------------------
        /**
          * isOneShot : read the 1SHOT
          *
          * @return true if the DS1621 is in one shot mode, false otherwise
          *
        */
        //---------------------------------------------------
        bool isOneShot(void);

        //---------------------------------------------------
        /**
          * setThresholdTemp : Set the low (TL) or high (TH) threshold temperature
          *
          * @param fTemp is the temperature to set
          * @param isLow if true set the low threshold, if false set the high
          *
          * @return the temperature that was set
          *
        */
        //---------------------------------------------------
        float setThresholdTemp(float fTemp,bool isLow);

        //---------------------------------------------------
        /**
          * getThresholdTemp : Get the low (TL) or high (TH) threshold temperature
          *
          * @param isLow if true get the low threshold, if false get the high
          *
          * @return the desired threshold temperature
        */
        //---------------------------------------------------
        float getThresholdTemp(bool isLow);


    private:
        // Flag to know if operations are valid or not
        bool m_isDeviceInitialized;

        // I2C Addres of the device
        unsigned char m_szAddres;

        // File descriptor of the device
        int m_nDeviceFD;

        //---------------------------------------------------
        /**
          * startStopConvert : Start or stop the temperature conversion
          *
          * @param isStop if true, stop the conversion, if false start it
          *
        */
        //---------------------------------------------------
        void startStopConvert(bool isStop);

        //---------------------------------------------------
        /**
          * waitEndOfConversion : Test configuration and return when conversion is done
          *
        */
        //---------------------------------------------------
        void waitEndOfConversion(void);

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
        float formatTemp(signed int nTemp);

        //---------------------------------------------------
        /**
          * setConfig : set device configuration
          *
          * @param szConfig is the configuration we want to set
          *
        */
        //---------------------------------------------------
        void setConfig(unsigned char szConfig);

        //---------------------------------------------------
        /**
          * getConfig : get device configuration
          *
          * @return the configuration of the device
          *
        */
        //---------------------------------------------------
        unsigned char getConfig(void);

        //---------------------------------------------------
        /**
          * writei2c : write at low level
          *
          * @param szData is the data to write
          *
        */
        //---------------------------------------------------
        void writei2c(unsigned char szData);

        //---------------------------------------------------
        /**
          * writeRegister8bitsi2c : write 8 bits data into a register at low level
          *
          * @param szRegister is the register to write
          * @param szData is the data to write
          *
        */
        //---------------------------------------------------
        void writeRegister8bitsi2c(unsigned char szRegister, unsigned char szData);

        //---------------------------------------------------
        /**
          * writeRegister16bitsi2c : write 16 bits data into a register at low level
          *
          * @param szRegister is the register to write
          * @param nData is the data to write
          *
        */
        //---------------------------------------------------
        void writeRegister16bitsi2c(unsigned char szRegister, int nData);

        //---------------------------------------------------
        /**
          * checkThresholdTemperatureRange : check temperature threshold value
          *
          * @param fTemp is the temperature to check
          *
        */
        //---------------------------------------------------
        void checkThresholdTemperatureRange(float &fTemp);

};
