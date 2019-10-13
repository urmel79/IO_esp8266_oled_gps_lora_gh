#include <Arduino.h>

#include "config.hpp"

#include "function_oled.hpp"

bool g_b_iicOLED_connected = false;
unsigned long g_ul_delayTime = 2000; // ms

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println();

  g_b_iicOLED_connected = connectOLEDiic();
}

void loop() {
  // updating the oled display non-blocking with Ticker does produce hard panic errors (esp8266 crashes)
  if (g_b_iicOLED_connected) printOLED_str("Das ist ein Test.");

  // printOLED_bme280_ccs811(g_d_bme280_temperature, g_i_bme280_humidity, g_d_bme280_pressure, g_i_bme280_altitude, g_ui_ccs811_eCO2, g_ui_ccs811_TVOC);

  delay(g_ul_delayTime);
}
