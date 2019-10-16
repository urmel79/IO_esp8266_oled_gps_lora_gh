#include <Arduino.h>

#include "config.hpp"

// include wifi functionality for asynchronous wifi handling
#include "function_wifi.hpp"

// include OTA functionality (ability for Over The Air updates)
#include "function_ota.hpp"

#include "function_oled.hpp"

#if !defined(D5)
  #define D5 (14)
  #define D6 (12)

  // if D8 ist connected to TxD during boot, there will be errors
  // => better use D5+D6 for serial
  #define D7 (13)
  #define D8 (15)
#endif

// Install EspSoftwareSerial by Peter Lerup
// Implementation of the Arduino software serial for ESP8266/ESP32.
// Important: v5.0.4 is the latest version what compiles without errors
// (current versions spill massive errors, maybe about interrupt handling?)
#include<SoftwareSerial.h> //Included SoftwareSerial Library
//Started SoftwareSerial at RX and TX pin of ESP8266/NodeMCU
SoftwareSerial serial_gps(D5, D6); // RxD: GPIO13 (D5), TxD: GPIO15 (D6)
#define BAUD_RATE 9600

#include <TinyGPS++.h>
// The TinyGPS++ object
// Examples:
// https://github.com/mkconer/ESP8266_GPS/blob/master/ESP8266_GPS_OLED_Youtube.ino
// https://circuitdigest.com/microcontroller-projects/interfacing-gps-with-nodemcu-esp12
TinyGPSPlus gps;

bool g_b_wifi_connected = false;
bool g_b_iicOLED_connected = false;
bool g_b_valid_gps_signal = false;
// unsigned long g_ul_delayTime = 2000; // ms

//variables for blinking an LED with Millis
const int g_i_led_pin = 2; // ESP8266 Pin to which onboard LED is connected
unsigned long g_ul_previousMillis = 0;  // will store last time LED was updated
const unsigned long g_ul_delayTime = 2000;  // interval at which to blink (milliseconds)
bool g_b_ledState = false;  // ledState used to set the LED

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  serial_gps.begin(BAUD_RATE);
  while (!serial_gps) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  pinMode(g_i_led_pin, OUTPUT);

  g_b_wifi_connected = connectToWifi();

  if (g_b_wifi_connected) function_ota_setup(HOSTNAME);   // setup OTA functionality

  g_b_iicOLED_connected = connectOLEDiic();
}

void loop() {
  function_ota_handle();  // call handler function for OTA

  // // while there is data coming in, read it
  // // and send to the hardware serial port:
  // while (serial_gps.available() > 0) {
	// 	Serial.write(serial_gps.read());
	// 	yield();
	// }

  // This sketch displays information every time a new sentence is correctly encoded.
  while (serial_gps.available() > 0){
    gps.encode(serial_gps.read());
    if (gps.location.isUpdated()){
      // Serial.print("Latitude= ");
      // Serial.print(gps.location.lat(), 6);
      // Serial.print(" Longitude= ");
      // Serial.println(gps.location.lng(), 6);
      g_b_valid_gps_signal = true;
    }
    else g_b_valid_gps_signal = false;

    // Yielding
    // This is one of the most critical differences between the ESP8266 and a more classical
    // Arduino microcontroller. The ESP8266 runs a lot of utility functions in the background –
    // keeping WiFi connected, managing the TCP/IP stack, and performing other duties.
    // Blocking these functions from running can cause the ESP8266 to crash and reset itself.
    // To avoid these mysterious resets, avoid long, blocking loops in your sketch.
    //
    // The amazing creators of the ESP8266 Arduino libraries also implemented a yield() function,
    // which calls on the background functions to allow them to do their things.
    yield();
  }

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
      // printOLED_end();

      // Serial.println("############## WIFI ##############");
      Serial.println(get_wifi_hostname());
      Serial.println(get_wifi_IP_str());
      Serial.println(get_wifi_RSSI());
      // Serial.println("############## WIFI ##############");

      if (!g_b_valid_gps_signal) {
        Serial.println("#### No GPS Signal .. ####");

        // printOLED_begin();
        printOLED_str(30, "# No GPS Signal .. #");
        printOLED_str(40, "");
        printOLED_str(50, "");
        printOLED_end();
      }
      else {
        // Serial.setCursor(0,0);
        Serial.print("Latitude  : ");
        Serial.print(gps.location.lat(), 5);
        Serial.print(char(176));                            // ° symbol
        Serial.println(" N");
        Serial.print("Longitude : ");
        Serial.print(gps.location.lng(), 4);
        Serial.print(char(176));                            // ° symbol
        Serial.println(" E");
        Serial.print("Satellites: ");
        Serial.println(gps.satellites.value());
        Serial.print("Elevation : ");
        Serial.print(gps.altitude.meters());
        Serial.println(" m");
        Serial.print("Time UTC  : ");
        Serial.print(gps.time.hour());                       // GPS time UTC
        Serial.print(":");
        Serial.print(gps.time.minute());                     // Minutes
        Serial.print(":");
        Serial.println(gps.time.second());                   // Seconds
        Serial.print("Heading   : ");
        Serial.print(gps.course.deg());
        Serial.println(char(176));                            // ° symbol
        // Serial.println(" °");
        Serial.print("Speed     : ");
        Serial.print(gps.speed.kmph());
        Serial.println(" km/h");

        // printOLED_begin();
        printOLED_values(30, "Lat.: ", String(gps.location.lat()));
        printOLED_values(40, "Long.: ", String(gps.location.lng()));
        printOLED_values(50, "Sat.: ", String(gps.satellites.value()));
        printOLED_end();
      }
    }
  }
}
