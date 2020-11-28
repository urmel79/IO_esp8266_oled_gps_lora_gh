#ifndef function_sht3x_hpp
#define function_sht3x_hpp

// Libs for SHT3x sensors
// #include <Wire.h>
#include <SPI.h>

// REQUIRES the following Arduino libraries:
// - SHT31-D Humidity & Temp Sensor Library: https://github.com/adafruit/Adafruit_SHT31
// Designed specifically to work with the SHT31-D sensor from Adafruit
// ----> https://www.adafruit.com/products/2857
// These sensors use I2C to communicate, 2 pins are required to interface
#include <Adafruit_SHT31.h>

/*
 * WeMos D1 Mini / NodeMCU uses pins for I2C bus:
 * D2 = SDA
 * D1 = SCL
 *
 * NodeMCU ESP32 I2C uses pins for I2C bus:
 * 21 = SDA
 * 22 = SCL
 *
 * Deauther Oled V5 uses pins for I2C bus:
 * D1 = SDA
 * D2 = SCL
*/
#if defined(WEMOS_D1_MINI_PRO) || defined(NODE_MCU_LORA_1) || defined(NODE_MCU_LORA_2)
  const int sdaPin = D2;
  const int sclPin = D1;
#elif defined(NODE_MCU_LORA_2_ESP32)
  const int sdaPin = 21;
  const int sclPin = 22;
#elif defined(DEAUTHER_OLED)
  const int sdaPin = D1;
  const int sclPin = D2;
#endif

// // create only a global pointer to the class Adafruit_SHT31 (for later instantiation)
// Adafruit_SHT31* ptr_sht31 = NULL;

bool function_SHTx_connect_iic();

void function_SHTx_enable_heater(bool enable);
bool function_SHTx_get_heater();

float function_SHTx_get_temperature();
float function_SHTx_get_humidity();








#endif // function_sht3x_hpp
