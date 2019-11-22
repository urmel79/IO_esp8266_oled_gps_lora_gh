#ifndef function_gps_hpp
#define function_gps_hpp

// install path: ~/.platformio/lib/TinyGPSPlus_ID1655/
#include <TinyGPS++.h>

// struct struct_gps_values {
//   struct TinyGPSLocation location;
//   struct TinyGPSDate date;
//   struct TinyGPSTime time;
//   struct TinyGPSSpeed speed;
//   struct TinyGPSCourse course;
//   struct TinyGPSAltitude altitude;
//   struct TinyGPSInteger satellites;
//   struct TinyGPSHDOP hdop;
// };

// struct struct_gps_values {
//   struct TinyGPSPlus s_gps;
//   bool valid = false;
// };

// struct struct_gps_values : public TinyGPSPlus{};

// typedef TinyGPSPlus struct_gps_values;

bool initSS_gps();

// struct_gps_values read_gps();

TinyGPSPlus read_gps();

#endif // function_gps_hpp
