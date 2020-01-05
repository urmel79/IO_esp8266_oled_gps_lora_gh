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
// Update to EspSoftwareSerial v6.4.0 (latest): API of begin() function has changed and no return value any more
#include <SoftwareSerial.h> //Included SoftwareSerial Library

#define GPS_BAUD_RATE 9600

// Install RunningAverage by Rob Tillaart
// The library stores the last N individual values in a circular buffer to calculate the running average.
// https://github.com/RobTillaart/Arduino/tree/master/libraries/RunningAverage
// #include "RunningAverage.h"
// use rewritten RunningAverage library (revert back  from float to double),
// because for smoothing GPS values I need much higher precision ...
#include "function_runningAverage_double.hpp"

#define GPS_AVG_FILTER_SIZE 60

RunningAverage gps_RunningAVG_lat(GPS_AVG_FILTER_SIZE);  // RunningAverage object for gps latitude
RunningAverage gps_RunningAVG_lng(GPS_AVG_FILTER_SIZE);  // RunningAverage object for gps longitude

struct_gps_RunningAVG_Median g_avg;

// #####################
// @TODO:
//        comparison between RunningAverage and RunningMedian => not so susceptible to outliers?
//        => library has to be rewritten first (revert back  from float to double)
// #####################
// RunningMedian by Rob Tillaart
// The library stores the last N individual values in a buffer to select the median. It filters outliers.
// https://github.com/RobTillaart/Arduino/tree/master/libraries/RunningMedian
#include "RunningMedian.h"

// #define GPS_AVG_FILTER_SIZE 100

RunningMedian gps_RunningMedian_lat(GPS_AVG_FILTER_SIZE);  // RunningMedian object for gps latitude
RunningMedian gps_RunningMedian_lng(GPS_AVG_FILTER_SIZE);  // RunningMedian object for gps longitude
// RunningMedian gps_RunningMedian_lat = RunningMedian(GPS_AVG_FILTER_SIZE);   // RunningMedian object for gps latitude
// RunningMedian gps_RunningMedian_lng = RunningMedian(GPS_AVG_FILTER_SIZE);   // RunningMedian object for gps longitude

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
// Some arduino boards only have one hardware serial port, so a software serial port is needed instead.
// comment out the above definition and uncomment these lines
// SoftwareSerial serial_gps(D3, D4); // RxD: GPIO0 (D3), TxD: GPIO2 (D4)
SoftwareSerial serial_gps;

// The TinyGPS++ object
// Examples:
// https://github.com/mkconer/ESP8266_GPS/blob/master/ESP8266_GPS_OLED_Youtube.ino
// https://circuitdigest.com/microcontroller-projects/interfacing-gps-with-nodemcu-esp12
TinyGPSPlus gps;

void initSS_gps() {
  // RxD: GPIO0 (D3), TxD: GPIO2 (D4)
  serial_gps.begin(GPS_BAUD_RATE, SWSERIAL_8N1, D3, D4, false, 95, 11);

  gps_RunningAVG_lat.clear(); // explicitly start with a clean RunningAverage object
  gps_RunningAVG_lng.clear(); // explicitly start with a clean RunningAverage object

  gps_RunningMedian_lat.clear(); // explicitly start with a clean RunningMedian object
  gps_RunningMedian_lng.clear(); // explicitly start with a clean RunningMedian object
}

// is called in every loop cycle => polling the serial line
TinyGPSPlus read_gps() {

  // TinyGPSPlus l_gps_values;

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

      // gps_RunningAVG_lat.addValue(gps.location.lat());
      // gps_RunningAVG_lng.addValue(gps.location.lng());

      // gps_RunningAVG_lat.addValue(gps.location.lat());
      // gps_RunningAVG_lng.addValue(gps.location.lng());
      //
      // gps_RunningMedian_lat.add(gps.location.lat());
      // gps_RunningMedian_lng.add(gps.location.lng());

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

// is called every 2 seconds in main loop
void gps_RunningAVG_Median_addValues() {
  gps_RunningAVG_lat.addValue(gps.location.lat());
  gps_RunningAVG_lng.addValue(gps.location.lng());

  gps_RunningMedian_lat.add(gps.location.lat());
  gps_RunningMedian_lng.add(gps.location.lng());
}

// is called every 2 seconds in main loop
struct_gps_RunningAVG_Median get_gps_RunningAVG_Median() {
  // struct_gps_RunningAVG_Median l_avg;

  g_avg.gps_RunningAVG_lat = gps_RunningAVG_lat.getAverage();
  g_avg.gps_RunningAVG_lng = gps_RunningAVG_lng.getAverage();

  g_avg.gps_RunningMedian_lat = gps_RunningMedian_lat.getMedian();
  g_avg.gps_RunningMedian_lng = gps_RunningMedian_lng.getMedian();

  g_avg.gps_RunningAVG_lat_size = gps_RunningAVG_lat.getSize();
  g_avg.gps_RunningAVG_lng_size = gps_RunningAVG_lng.getSize();
  g_avg.gps_RunningAVG_lat_cnt = gps_RunningAVG_lat.getCount();
  g_avg.gps_RunningAVG_lng_cnt = gps_RunningAVG_lng.getCount();

  return g_avg;
}










//
