/*
Written by Nejc Klemenčič, December 22nd, 2019

This is a library for the SCD30 CO2 Sensor Module. 
The sensor uses either I2C or UART to comminucate.
This library is intented for the I2C interface.

The library with the I2C interface can be found at: <insert later>

The SCD30 measures CO2 with an accuracy of +/- 30ppm.

This library handles the initialization of the SCD30
and outputs CO2, humidity and temperature levels.

Sensor interface description can be found at: 
https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/9.5_CO2/Sensirion_CO2_Sensors_SCD30_Interface_Description.pdf.
*/

#ifndef SCD30_UART_lib
#define SCD30_UART_lib

#if (ARDUINO >= 100)
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif

#define SCD30_UART_ADDRESS 0x61 //default SCD30 UART address

//function codes
#define SCD30_READ_HOLDING_REGISTERS 3
#define SCD30_WRITE_SINGLE_HOLDING_REGISTER 6

//defines for available commands
#define SCD30_START_CONTINUOUS_MEASUREMENT 0x0036
#define SCD30_STOP_CONTINUOUS_MEASUREMENT 0x0037
#define SCD30_SET_MEASUREMENT_INTERVAL 0x0025
#define SCD30_GET_READY_STATUS 0x0027
#define SCD30_READ_MEASUREMENT 0x0028
#define SCD30_SET_ALTITUDE_COMPENSATION 0x0038
#define SCD30_SET_TEMPERATURE_OFFSET 0x003B
#define SCD30_SET_AUTOMATIC_SELFCALIBRATION 0x003A
#define SCD30_SET_FORCED_RECALIBRATION 0x0039
#define SCD30_READ_FIRMWARE_VERSION 0x0038

//the supported baud rate is 19200 Baud with 8 Data bits, 1 Start bit and 1 Stop bit, no Parity bit
#define BAUD_RATE 19200

class SCD30 
{
    public:
        SCD30(); //constructor
        ~SCD30(); //destructor

        void begin(); //initialize library instance

        void beginMeasuring(void); //starts the measurements, with the default interval of 2s
        void beginMeasuring(uint16_t ambientPressureOffset); //starts the measurements with ambient pressure copensation in mBar, 
                                                                //if argument is 0, pressure compensation is deactivated
        void stopMeasuring(); //stops the measurements

        boolean dataAvailable(); //checks if data is available

        void enableAutomaticSelfCalibration(); //enables automatic self calibration
        void disableAutomaticSelfCalibration(); //disables automatic self calibration
       
        void setForcedRecalibrationValue(uint16_t concentration); //sets a reference CO2 value, values range from 400 ppm to 2000 ppm
        void setMeasurementInterval(uint16_t interval); //changes the default measurement interval
        void setTemperatureOffset(float tempOffset); //sets temperature offset for onboard RH/T sensor
        void setAmbientPressure(uint16_t abmientPressure); //sets ambientPressure, values range from 700 to 1200 mBar
        void setAltitudeCompensation(uint16_t altitude); //sets altitude, values range from 0 upwards, in meters
        
        void readMeasurement(); //reads 17 byte measurement

        float getHumidity(); //gets humidity in %RH
        float getTemperatureC(); //gets temperature in °C
        float getTemperatureF(); //gets temperature in F
        float getTemperatureK(); //gets temperature in K
        uint16_t getCO2(); //gets CO2 concentration in ppm

        void sendCommand(uint8_t functionCode, uint16_t command, uint16_t argument); //sends command via UART, with an additional argument
        void clearBuffer(); //clears serial buffer

        uint16_t computeCRC16(uint8_t data[], uint8_t len); //calculates crc checksum
        
        uint8_t* getFirmwareVersion();

    private:
        //measured values
        float co2 = 0;
        float temperature = 0;
        float humidity = 0;
        uint8_t* firmwareVersion;
};

#endif