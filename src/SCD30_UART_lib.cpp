/*
Written by Nejc Klemenčič, December 22th, 2019

This is a library for the SCD30 CO2 Sensor Module. 
The sensor uses either I2C or UART to comminucate.
This library is intented for the UART interface.

The library with the I2C interface can be found at: <insert later>


The SCD30 measures CO2 with an accuracy of +/- 30ppm.

This library handles the initialization of the SCD30
and outputs CO2, humidity and temperature levels.

Sensor interface description can be found at: 
https://www.sensirion.com/fileadmin/user_upload/customers/sensirion/Dokumente/9.5_CO2/Sensirion_CO2_Sensors_SCD30_Interface_Description.pdf.
*/
#include "SCD30_UART_lib.h"

SCD30::SCD30()
{
    //constructor
}

SCD30::~SCD30()
{
    //destructor
}

void SCD30::begin()
{
    //the supported baud rate is 19200 Baud with 8 Data bits, 1 Start bit and 1 Stop bit, no Parity bit
    Serial1.begin(BAUD_RATE);

    beginMeasuring(); //start continuous measurements
    setMeasurementInterval(2); //2 seconds between measurements - default value
}

//begins continuous measurements, status is saved in non-volatile memory
//the device continues measuring after repowering without sending the measurement command 
//returns true if successful
//begin measuring without pressureOffset
void SCD30::beginMeasuring(void)
{
    beginMeasuring(0); //triggers continuous measurement, argument = 0 deactivates pressure compensation
}

//overload 
//see 1.4.1 in document
void SCD30::beginMeasuring(uint16_t ambientPressureOffset)
{
    sendCommand(SCD30_WRITE_SINGLE_HOLDING_REGISTER, SCD30_START_CONTINUOUS_MEASUREMENT, ambientPressureOffset);
    clearBuffer();
}

//stops continuous measuring, measuring can be resumed with beginMeasuring
//see 1.4.2 in document
void SCD30::stopMeasuring()
{
    sendCommand(SCD30_WRITE_SINGLE_HOLDING_REGISTER, SCD30_STOP_CONTINUOUS_MEASUREMENT, 1);
    clearBuffer();
}

//returns true when data from the sensor is available
//see 1.4.4 in document
boolean SCD30::dataAvailable()
{
    sendCommand(SCD30_READ_HOLDING_REGISTERS, SCD30_GET_READY_STATUS, 1);
    delay(100);
    uint8_t byteCounter = 0;
    boolean dataAvailable = false;
    while (Serial1.available() > 0)
    {
        uint8_t data = Serial1.read();
        if(byteCounter == 4 && data == 1) //5th byte of response is 1 when data is available
            dataAvailable = true;
        byteCounter++;
    }

    if (dataAvailable == true) return true;
    else return false;
}

//enables automatic self calibration 
//see 1.4.5 in document
void SCD30::enableAutomaticSelfCalibration()
{
    sendCommand(SCD30_WRITE_SINGLE_HOLDING_REGISTER, SCD30_SET_AUTOMATIC_SELFCALIBRATION, 1);
    clearBuffer();
}

//disables automatic self calibration 
//see 1.4.5 in document
void SCD30::disableAutomaticSelfCalibration()
{
    sendCommand(SCD30_WRITE_SINGLE_HOLDING_REGISTER, SCD30_SET_AUTOMATIC_SELFCALIBRATION, 0);
    clearBuffer();
}

//sets reference CO2 concentration in ppm
//valid values in range from 400 to 2000 ppm
//see 1.4.5 in document
void SCD30::setForcedRecalibrationValue(uint16_t concentration)
{
    if (concentration < 400 || concentration > 2000) return;
    sendCommand(SCD30_WRITE_SINGLE_HOLDING_REGISTER, SCD30_SET_FORCED_RECALIBRATION, concentration);
    clearBuffer();
}

//sets measurement interval
//valid values in range from 2 seconds to 1800 seconds (30 minutes)
//see 1.4.3 in document
void SCD30::setMeasurementInterval(uint16_t interval)
{
    sendCommand(SCD30_WRITE_SINGLE_HOLDING_REGISTER, SCD30_SET_MEASUREMENT_INTERVAL, interval);
    clearBuffer();
}

//sets temperature offset
//see 1.4.6 in document
void SCD30::setTemperatureOffset(float tempOffset)
{
    uint16_t tickOffest = tempOffset * 100;
    sendCommand(SCD30_WRITE_SINGLE_HOLDING_REGISTER, SCD30_SET_TEMPERATURE_OFFSET, tickOffest);
    clearBuffer();
}

//sets ambient pressure after initialization
//ambient pressure can also be set with beginMeasuring(uint16_t ambientPressureOffset)
//valid values in range from 700 to 1200 mBar
//see 1.4.1 in document
void SCD30::setAmbientPressure(uint16_t ambientPressure)
{
    if (ambientPressure < 700 || ambientPressure > 1200) 
    {
        ambientPressure = 0;
    }

    sendCommand(SCD30_WRITE_SINGLE_HOLDING_REGISTER, SCD30_START_CONTINUOUS_MEASUREMENT, ambientPressure);
    clearBuffer();
}

//sets altitude compensation, valid values range from 0 upwards, in m
//see 1.4.7 in document
void SCD30::setAltitudeCompensation(uint16_t altitude)
{
    sendCommand(SCD30_WRITE_SINGLE_HOLDING_REGISTER, SCD30_SET_ALTITUDE_COMPENSATION, altitude);
    clearBuffer();
}

//reads 17 bytes from sensor
//updates global variables with read value
//see 1.4.4 in document
void SCD30::readMeasurement()
{
    uint32_t tempCO2 = 0;
    uint32_t tempHumidity = 0;
    uint32_t tempTemperature = 0;
    uint8_t byteCounter = 0;

    sendCommand(SCD30_READ_HOLDING_REGISTERS, SCD30_READ_MEASUREMENT, 6); //6 is the number of holding registers to read
    delay(100); //set the delay to allow sensor to respond

    while (Serial1.available() > 0)
    {
        uint8_t incoming = Serial1.read();
        switch(byteCounter) 
        {
            case 3:
            case 4:
            case 5:
            case 6:
                tempCO2 <<= 8;
                tempCO2 |= incoming;
                break;

            case 7:
            case 8:
            case 9:
            case 10:
                tempTemperature <<= 8;
                tempTemperature |= incoming;
                break;

            case 11:
            case 12:
            case 13:
            case 14:
                tempHumidity <<= 8;
                tempHumidity |= incoming;
                break;
            default:
                //skip all CRC bytes
                break;
        }
        byteCounter++;
    }
    
    //copy the uint32_t CO2 values into uint16_t
    memcpy(&co2, &tempCO2, sizeof(co2));
    //copy the uint32_t temperature and humidity into their associated floats
    memcpy(&temperature, &tempTemperature, sizeof(temperature));
    memcpy(&humidity, &tempHumidity, sizeof(humidity));
}

//returns latest available humidity
float SCD30::getHumidity() 
{
    //check if new data is available to read, if not return currently saved data
    if (dataAvailable() == true)
        readMeasurement();
    
    return humidity;
}

//returns latest available temperature in °C
float SCD30::getTemperatureC()
{
    //check if new data is available to read, if not return currently saved data
    if (dataAvailable() == true)
        readMeasurement();
    
    return temperature;
}

//returns latest available temperature in F
float SCD30::getTemperatureF()
{
    //check if new data is available to read, if not return currently saved data
    if (dataAvailable() == true)
        readMeasurement();
    
    return temperature * 1.8 + 32;
}

//returns latest available temperature in K
float SCD30::getTemperatureK()
{
    //check if new data is available to read, if not return currently saved data
    if (dataAvailable() == true)
        readMeasurement();
    
    return temperature + 273.15;
}
//returns latest available CO2 level
uint16_t SCD30::getCO2()
{
    //check if new data is available to read, if not return currently saved data
    if (dataAvailable() == true)
        readMeasurement();
    
    return co2;
}

//sends a command with an argument
//we need to calculate CRC on the entire request
void SCD30::sendCommand(uint8_t functionCode, uint16_t address, uint16_t argument)
{
    uint8_t addressMSB = address >> 8;
    uint8_t addressLSB = address & 0xFF;
    uint8_t argumentMSB = argument >> 8;
    uint8_t argumentLSB = argument & 0xFF;
    uint8_t data[8] = {SCD30_UART_ADDRESS, functionCode, addressMSB, addressLSB, argumentMSB, argumentLSB, 0, 0}; //we put the last two values at 0 for now
                                                                                                                   //because we have to compute crc on all of the data
    uint16_t crc = computeCRC16(data, 6); //calculate CRC on all data
    uint8_t crcLSB = crc >> 8;
    uint8_t crcMSB = crc & 0xFF;
    
    data[6] = crcLSB;
    data[7] = crcMSB;

    Serial1.write(data, 8);
    Serial1.flush();
}

//used to clear the response buffer
//used only when we don't read the response data
void SCD30::clearBuffer() 
{
    delay(100); //we set a delay so the sensor has enough time to respond
    while (Serial1.available() > 0)
    {
        Serial1.read();
    }
}

/*
calculates crc on all of the data being sent
crc needs to be included as last two bytes of the request
http://www.modbus.org/docs/Modbus_over_serial_line_V1_02.pdf, chapter 6.2.2
http://modbus.org/docs/PI_MBUS_300.pdf pages 114, 115
*/
uint16_t SCD30::computeCRC16(uint8_t data[], uint8_t len)
{
    /* Table of CRC values for high–order byte */
    static uint8_t tableCRCMSB[] = {
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01,
    0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0,
    0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01,
    0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81, 0x40, 0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41,
    0x00, 0xC1, 0x81, 0x40, 0x01, 0xC0, 0x80, 0x41, 0x01, 0xC0, 0x80, 0x41, 0x00, 0xC1, 0x81,
    0x40
    };

    /* Table of CRC values for low–order byte */
    static uint8_t tableCRCLSB[] = {
    0x00, 0xC0, 0xC1, 0x01, 0xC3, 0x03, 0x02, 0xC2, 0xC6, 0x06, 0x07, 0xC7, 0x05, 0xC5, 0xC4,
    0x04, 0xCC, 0x0C, 0x0D, 0xCD, 0x0F, 0xCF, 0xCE, 0x0E, 0x0A, 0xCA, 0xCB, 0x0B, 0xC9, 0x09,
    0x08, 0xC8, 0xD8, 0x18, 0x19, 0xD9, 0x1B, 0xDB, 0xDA, 0x1A, 0x1E, 0xDE, 0xDF, 0x1F, 0xDD,
    0x1D, 0x1C, 0xDC, 0x14, 0xD4, 0xD5, 0x15, 0xD7, 0x17, 0x16, 0xD6, 0xD2, 0x12, 0x13, 0xD3,
    0x11, 0xD1, 0xD0, 0x10, 0xF0, 0x30, 0x31, 0xF1, 0x33, 0xF3, 0xF2, 0x32, 0x36, 0xF6, 0xF7,
    0x37, 0xF5, 0x35, 0x34, 0xF4, 0x3C, 0xFC, 0xFD, 0x3D, 0xFF, 0x3F, 0x3E, 0xFE, 0xFA, 0x3A,
    0x3B, 0xFB, 0x39, 0xF9, 0xF8, 0x38, 0x28, 0xE8, 0xE9, 0x29, 0xEB, 0x2B, 0x2A, 0xEA, 0xEE,
    0x2E, 0x2F, 0xEF, 0x2D, 0xED, 0xEC, 0x2C, 0xE4, 0x24, 0x25, 0xE5, 0x27, 0xE7, 0xE6, 0x26,
    0x22, 0xE2, 0xE3, 0x23, 0xE1, 0x21, 0x20, 0xE0, 0xA0, 0x60, 0x61, 0xA1, 0x63, 0xA3, 0xA2,
    0x62, 0x66, 0xA6, 0xA7, 0x67, 0xA5, 0x65, 0x64, 0xA4, 0x6C, 0xAC, 0xAD, 0x6D, 0xAF, 0x6F,
    0x6E, 0xAE, 0xAA, 0x6A, 0x6B, 0xAB, 0x69, 0xA9, 0xA8, 0x68, 0x78, 0xB8, 0xB9, 0x79, 0xBB,
    0x7B, 0x7A, 0xBA, 0xBE, 0x7E, 0x7F, 0xBF, 0x7D, 0xBD, 0xBC, 0x7C, 0xB4, 0x74, 0x75, 0xB5,
    0x77, 0xB7, 0xB6, 0x76, 0x72, 0xB2, 0xB3, 0x73, 0xB1, 0x71, 0x70, 0xB0, 0x50, 0x90, 0x91,
    0x51, 0x93, 0x53, 0x52, 0x92, 0x96, 0x56, 0x57, 0x97, 0x55, 0x95, 0x94, 0x54, 0x9C, 0x5C,
    0x5D, 0x9D, 0x5F, 0x9F, 0x9E, 0x5E, 0x5A, 0x9A, 0x9B, 0x5B, 0x99, 0x59, 0x58, 0x98, 0x88,
    0x48, 0x49, 0x89, 0x4B, 0x8B, 0x8A, 0x4A, 0x4E, 0x8E, 0x8F, 0x4F, 0x8D, 0x4D, 0x4C, 0x8C,
    0x44, 0x84, 0x85, 0x45, 0x87, 0x47, 0x46, 0x86, 0x82, 0x42, 0x43, 0x83, 0x41, 0x81, 0x80,
    0x40
    };

    uint8_t crcMSB = 0xFF; //initialize with 0xFF
    uint8_t crcLSB = 0xFF;
    uint8_t index;
    uint8_t dataIndex = 0;

    while (len--) /* pass through message buffer */
    {
        index = crcMSB ^ data[dataIndex++]; /* calculate the CRC */
        crcMSB = crcLSB ^ tableCRCMSB[index];
        crcLSB = tableCRCLSB[index];
    }
    return (crcMSB << 8 | crcLSB);
}

//gets current firmware version, formatted as [Major, Minor]
//see 1.4.8 in document
uint8_t* SCD30::getFirmwareVersion()
{
    if (sizeof(firmwareVersion) == 0) {
        uint8_t byteCounter = 0;
        uint8_t version[2] = {0, 0};

        while (Serial1.available() > 0)
        {
            uint8_t incoming = Serial1.read();
            if (byteCounter == 3) version[0] = incoming; //firmware version data is stored in bytes 4 and 5
            if (byteCounter == 4) version[1] = incoming;

            byteCounter++;
        }

        firmwareVersion = version;
    }

    else clearBuffer();
    return firmwareVersion;
}