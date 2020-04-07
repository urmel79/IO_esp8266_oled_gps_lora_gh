#ifndef function_gps_hpp
#define function_gps_hpp

// install path: ~/.platformio/lib/TinyGPSPlus_ID1655/
#include <TinyGPS++.h>

#if defined(NODE_MCU_LORA_1) || defined(NODE_MCU_LORA_2)
  const int RxD_Pin = D3;   // RxD: GPIO0 (D3) => TxD of GPS
  const int TxD_Pin = D4;   // TxD: GPIO2 (D4) => RxD of GPS
#elif defined(NODE_MCU_LORA_2_ESP32)
  const int RxD_Pin = 25;   // RxD: GPIO25 => TxD of GPS
  const int TxD_Pin = 26;   // TxD: GPIO26 => RxD of GPS
#endif

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
