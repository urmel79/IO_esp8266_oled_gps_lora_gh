#include "function_gps.hpp"

// Install EspSoftwareSerial by Peter Lerup
// Implementation of the Arduino software serial for ESP8266/ESP32.
// Important: v5.0.4 is the latest version what compiles without errors
// (current versions spill massive errors, maybe about interrupt handling?)
#include<SoftwareSerial.h> //Included SoftwareSerial Library

#include <TinyGPS++.h>

#define BAUD_RATE 9600

#if !defined(D5)
  #define D5 (14)
  #define D6 (12)

  // if D8 ist connected to TxD during boot, there will be errors
  // => better use D5+D6 for serial
  #define D7 (13)
  #define D8 (15)
#endif

// Started SoftwareSerial at RX and TX pin of ESP8266/NodeMCU
SoftwareSerial serial_gps(D5, D6); // RxD: GPIO13 (D5), TxD: GPIO15 (D6)

// The TinyGPS++ object
// Examples:
// https://github.com/mkconer/ESP8266_GPS/blob/master/ESP8266_GPS_OLED_Youtube.ino
// https://circuitdigest.com/microcontroller-projects/interfacing-gps-with-nodemcu-esp12
TinyGPSPlus gps;

bool initSS_gps() {
  bool status;

  status = serial_gps.begin(BAUD_RATE);
  if (!serial_gps) Serial.println("Something is wrong with the software serial line to GPS module.");

  return status;
}

void read_gps() {
  ;
}
