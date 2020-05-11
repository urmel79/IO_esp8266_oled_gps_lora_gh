#ifndef config_hpp
#define config_hpp

// Include your own wifi credentials here ...
// ... 'WIFI_SSID' and 'WIFI_PASSWORD' has to be defined there by yourself.
#include "wifi_creds.hpp"

// choose hostname and sensors (done by build_flags in platformio.ini)
#ifdef WEMOS_D1_MINI_PRO
  #define HOSTNAME "Wemos_D1_mini_pro"
  #define BOX_HAS_CCS811  // CO2-Sensor
  // #define BOX 'esp8266_d1_box'
#elif NODE_MCU_LORA_1
  #define HOSTNAME "LoRaNode_1"
  // #define BOX_HAS_PIR     // PIR Bewegungsmelder
  // #define BOX_HAS_CCS811  // CO2-Sensor
  // #define BOX 'esp8266_node_mcu_lora_1'
#elif NODE_MCU_LORA_2
  #define HOSTNAME "LoRaNode_2"
  // #define BOX_HAS_PIR     // PIR Bewegungsmelder
  // #define BOX_HAS_CCS811  // CO2-Sensor
  // #define BOX 'esp8266_node_mcu_lora_2'
#elif NODE_MCU_LORA_2_ESP32
  #define HOSTNAME "LoRaNode_2_ESP32"
  // #define BOX_HAS_PIR     // PIR Bewegungsmelder
  // #define BOX_HAS_CCS811  // CO2-Sensor
  // #define BOX 'esp32_node_mcu_lora_2'
#elif DEAUTHER_OLED
  #define HOSTNAME "Deauther_oled"
  // #define BOX 'esp8266_deauther_oled'
#endif

// choose mqtt broker (done by build_flags in platformio.ini)
#ifdef MQTT_RASPI_TOUCH
  //#define MQTT_HOST IPAddress(192, 168, 60, 190)
  #define MQTT_HOST "RaspiB3PiTouch7"
  #define MQTT_PORT 1883
#elif MQTT_RASPI_EPAPER
  #define MQTT_HOST "RaspiB3plus_ePaper"
  #define MQTT_PORT 1883
#elif MQTT_RASPI_LORAGW
  #define MQTT_HOST "RaspiB3plusLoRaGateway"
  #define MQTT_PORT 1883
#endif

// define mqtt root topics
#ifdef WEMOS_D1_MINI_PRO
  #define MQTT_ROOT_TOPIC "d1_box"
#elif NODE_MCU_LORA_1
  #define MQTT_ROOT_TOPIC "lora_1"
  #define MQTT_SENSOR_NODE "lora_1_gps"
  #define GPS_COLOR "Yellow"
#elif NODE_MCU_LORA_2
  #define MQTT_ROOT_TOPIC "lora_2"
  #define MQTT_SENSOR_NODE "lora_2_gps"
  #define GPS_COLOR "Blue"
#elif NODE_MCU_LORA_2_ESP32
  #define MQTT_ROOT_TOPIC "lora_2_esp32"
  #define MQTT_SENSOR_NODE "lora_2_gps_esp32"
  #define GPS_COLOR "Blue"
#elif NODE_MCU_V2
  #define MQTT_ROOT_TOPIC "node_mcu"
#elif DEAUTHER_OLED
  #define MQTT_ROOT_TOPIC "deauther_oled"
#endif

// LoRa topology: decide between Peer-to-Peer sending between 2 LoRa nodes
// and sending in an LoRaWAN network using a LoRaWAN gateway and The Things Network (TTN)
#ifdef LORA_TOPO_P2P
  #warning "Chosen topology for LoRa is Peer-to-Peer"
  #define LORA_OLED_P2P

  // define LoRa sender or receiver role
  #ifdef NODE_MCU_LORA_1
    #define LORA_SENDER
  #elif NODE_MCU_LORA_2 || NODE_MCU_LORA_2_ESP32
    #define LORA_RECEIVER
  #endif
#elif LORA_TOPO_TTN
  #warning "Chosen topology for LoRa is LoRaWAN and TTN"
  #define LORA_OLED_TTN
#else
  #error "Please choose a LoRa or LoRaWAN topology!"
#endif











#endif // config_hpp
