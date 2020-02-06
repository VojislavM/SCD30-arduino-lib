# UART Arduino Library for CO₂ Humidity and Temperature Sensor - SCD30
[CO₂ Humidity and Temperature Sensor - SCD30](https://www.sensirion.com/en/environmental-sensors/carbon-dioxide-sensors-co2/)

The SCD30 from Sensirion is a high quality Nondispersive Infrared (NDIR) based CO₂ sensor capable of detecting 400 to 10000ppm with an accuracy of ±(30ppm+3%). In order to improve accuracy the SCD30 has temperature and humidity sensing built-in, as well as commands to set the current altitude. For additional accuracy the SCD30 also accepts ambient pressure readings.

Note: The SCD30 has an automatic self-calibration routine. Sensirion recommends 7 days of continuous readings with at least 1 hour a day of 'fresh air' for self-calibration to complete.

I've written an Arduino library to make CO₂, humidity, and temperature readings easy. Additionaly, settings can be set and read.
To select the UART interface the SEL pin needs to be pulled to VDD voltage during power-up of the SCD30 sensor module.

NOTE: This library is intended for use only with Hardware Serial, the default is Serial1.

There is also an I2C version of this library which can be found here: https://github.com/NejcKle/sens1.

The library was written with the help of [SparkFun_SCD30_Arduino_Library](https://github.com/sparkfun/SparkFun_SCD30_Arduino_Library).

Library written by Nejc Klemenčič.

## Repository Contents
* /examples - Example sketches for the library (.ino). Run these from the Arduino IDE.
* /src - Source files for the library (.cpp, .h).
* keywords.txt - Keywords from this library that will be highlighted in the Arduino IDE.
* library.properties - General library properties for the Arduino package manager.
