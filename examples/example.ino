/*
Written by Nejc Klemenčič, December 22th, 2019

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

#include <Wire.h>
#include "SCD30_UART_lib.h"

SCD30 scdSensor;

void setup()  
{
  //connect TX pin from sensor to RX pin on board
  //connect RX pin from sensor to TX pin on board
  //SELECT pin on sensor has to be pulled to VDD
  Serial.begin(19200);
  Serial.println("SCD30 Example");
  scdSensor.begin(); //this will cause readings to occur every two seconds
  scdSensor.setMeasurementInterval(10); //we want to change that to 10 seconds

  scdSensor.setAltitudeCompensation(240); //set altitude in m

  scdSensor.setAmbientPressure(1020); //set pressure in mBar
}

void loop()  
{
  Serial.print("co2(ppm):");
  Serial.print(scdSensor.getCO2());

  Serial.print(" temp(C):");
  Serial.print(scdSensor.getTemperatureC(), 1);

  Serial.print(" humidity(%):");
  Serial.print(scdSensor.getHumidity(), 1);
  Serial.println();

  Serial.print("firmware version: ");
  Serial.print(scdSensor.getFirmwareVersion()[0], DEC);
  Serial.print(".");
  Serial.println(scdSensor.getFirmwareVersion()[1], DEC);

  delay(2000); //print values every two seconds
}