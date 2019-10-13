#include <Arduino.h>

#include "config.hpp"

// include wifi functionality for asynchronous wifi handling
#include "function_wifi.hpp"

// include OTA functionality (ability for Over The Air updates)
#include "function_ota.hpp"

#include "function_oled.hpp"

bool g_b_wifi_connected = false;
bool g_b_iicOLED_connected = false;
// unsigned long g_ul_delayTime = 2000; // ms

//variables for blinking an LED with Millis
const int g_i_led_pin = 2; // ESP8266 Pin to which onboard LED is connected
unsigned long g_ul_previousMillis = 0;  // will store last time LED was updated
const unsigned long g_ul_delayTime = 500;  // interval at which to blink (milliseconds)
bool g_b_ledState = false;  // ledState used to set the LED

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println();

  pinMode(g_i_led_pin, OUTPUT);

  g_b_wifi_connected = connectToWifi();

  if (g_b_wifi_connected) function_ota_setup(HOSTNAME);   // setup OTA functionality

  g_b_iicOLED_connected = connectOLEDiic();
}

void loop() {
  function_ota_handle();  // call handler function for OTA

  // loop to blink without delay
  unsigned long currentMillis = millis();
  if (currentMillis - g_ul_previousMillis >= g_ul_delayTime) {
    // save the last time you blinked the LED
    g_ul_previousMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
    g_b_ledState = not(g_b_ledState);
    // set the LED with the ledState of the variable:
    digitalWrite(g_i_led_pin,  g_b_ledState);

    // updating the oled display non-blocking with Ticker does produce hard panic errors (esp8266 crashes)
    if (g_b_iicOLED_connected) {
      printOLED_begin();
      printOLED_values(0, "Host: ", get_wifi_hostname());
      printOLED_values(10, "IP: ", get_wifi_IP_str());
      printOLED_values(20, "RSSI: ", String(get_wifi_RSSI()));
      printOLED_end();
    }
  }
}
