#ifndef function_gps_hpp
#define function_gps_hpp

// install path: ~/.platformio/lib/TinyGPSPlus_ID1655/
#include <TinyGPS++.h>

struct struct_gps_RunningAVG_Median {
  float gps_RunningAVG_lat;
  float gps_RunningAVG_lng;
  float gps_RunningMedian_lat;
  float gps_RunningMedian_lng;

  uint8_t gps_RunningAVG_lat_size;
  uint8_t gps_RunningAVG_lng_size;
  uint8_t gps_RunningAVG_lat_cnt;
  uint8_t gps_RunningAVG_lng_cnt;
};

void initSS_gps();

TinyGPSPlus read_gps();

void gps_RunningAVG_Median_addValues();
struct_gps_RunningAVG_Median get_gps_RunningAVG_Median();

#endif // function_gps_hpp
