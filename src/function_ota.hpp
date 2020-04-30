#ifndef function_ota_hpp
#define function_ota_hpp

#include <ArduinoOTA.h>

#if defined(WEMOS_D1_MINI_PRO) || defined(NODE_MCU_LORA_1) || defined(NODE_MCU_LORA_2)
  #define OTA_PASSWORD "esp8266"
#elif defined(NODE_MCU_LORA_2_ESP32)
  #define OTA_PASSWORD "esp32"
#endif

#define OTA_ESP_PORT 8266

void function_ota_setup( const char *chr_hostname );

void function_ota_handle( void );

#endif // function_ota_hpp
