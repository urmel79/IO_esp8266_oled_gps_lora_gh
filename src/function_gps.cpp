#include "function_gps.hpp"

// includes implicit <sys/types.h> (necessary for SoftwareSerial)
// otherwise errors like 'ssize_t' does not name a type'
#include <Arduino.h>

// Install EspSoftwareSerial by Peter Lerup
// Implementation of the Arduino software serial for ESP8266/ESP32.
// Important: v5.0.4 is the latest version what compiles without errors
// (current versions spill massive errors, maybe about interrupt handling?)
// Update: v5.3.4 does compile again without errors (@TODO: integration test pending)
//          => does not compile in other projects => why? => so going back to v5.0.4
#include <SoftwareSerial.h> //Included SoftwareSerial Library

#define BAUD_RATE 9600

#if !defined(D5)
  #define D3 (0)
  #define D4 (2)

  #define D5 (14)
  #define D6 (12)

  // if D8 ist connected to TxD during boot, there will be errors
  // => better use D5+D6 for serial
  #define D7 (13)
  #define D8 (15)
#endif

// Started SoftwareSerial at RX and TX pin of ESP8266/NodeMCU
// SoftwareSerial serial_gps(D5, D6); // RxD: GPIO13 (D5), TxD: GPIO15 (D6)
SoftwareSerial serial_gps(D3, D4); // RxD: GPIO0 (D3), TxD: GPIO2 (D4)

// The TinyGPS++ object
// Examples:
// https://github.com/mkconer/ESP8266_GPS/blob/master/ESP8266_GPS_OLED_Youtube.ino
// https://circuitdigest.com/microcontroller-projects/interfacing-gps-with-nodemcu-esp12
TinyGPSPlus gps;

bool initSS_gps() {
  bool status;

  status = serial_gps.begin(BAUD_RATE);
  if (!status) Serial.println("Something is wrong with the software serial line to GPS module.");

  return status;
}

// struct gps_values read_gps() {
TinyGPSPlus read_gps() {

  TinyGPSPlus l_gps_values;

  // // Only for debugging the gps:
  // // while there is data coming in, read it
  // // and send to the hardware serial port:
  // while (serial_gps.available() > 0) {
  // 	Serial.write(serial_gps.read());
  // 	yield();
  // }

  // This sketch displays information every time a new sentence is correctly encoded.
  while (serial_gps.available() > 0) {

    gps.encode(serial_gps.read());

    if (gps.location.isValid()) {

      // l_gps_values.location.lat = gps.location.lat();
      // l_gps_values.location.lng = gps.location.lng();
      // g_gps_values.date = gps.date;
      // g_gps_values.time = gps.time;
      // g_gps_values.speed = gps.speed;
      // g_gps_values.course = gps.course;
      // g_gps_values.altitude = gps.altitude;
      // g_gps_values.satellites = gps.satellites;
      // g_gps_values.hdop = gps.hdop;

      // make a local (shallow) copy of gps structure, just like with memcpy.
      // That means if you have e.g. a structure containing pointers, it's only the actual
      // pointers that will be copied and not what they point to, so after the copy you
      // will have two pointers pointing to the same memory.
      // l_gps_values = gps;

      return gps;
    }

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

  return gps;
  // return l_gps_values;
}
