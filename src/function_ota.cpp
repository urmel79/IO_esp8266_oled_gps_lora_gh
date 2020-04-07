#include "function_ota.hpp"

// 'ESP32' and 'ESP8266' are defined within PlatformIO
#ifdef ESP32
  #warning "ESP8266 was defined: doing ESP8266 stuff"
  #include "rom/ets_sys.h"
  // #include <ets_sys_bk.hpp>
  #define ETS_GPIO_INUM       12
#endif

void function_ota_setup( const char *chr_hostname ) {
  // Port defaults to 8266
  ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(chr_hostname);

  // No authentication by default
  ArduinoOTA.setPassword(OTA_PASSWORD);

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    // [bug] external interrupts conflict with wifi connection attempts and
    // ota updates and lead to sporadic crashes!
    // [fix (workaround)] disable external interrupts temporarily
    // reference https://www.mikrocontroller.net/topic/460256#5573848
    // ETS_GPIO_INTR_DISABLE();
    ets_isr_mask((1<<ETS_GPIO_INUM)); // disable interrupts by external GPIOs

    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    // enable external interrupts again
    // ETS_GPIO_INTR_ENABLE();
    ets_isr_unmask((1<<ETS_GPIO_INUM)); // enable interrupts by external GPIOs

    Serial.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    // disable external interrupts (they cause sporadic crashes!)
    // reference https://www.mikrocontroller.net/topic/460256#5573848
    // ETS_GPIO_INTR_DISABLE();
    ets_isr_mask((1<<ETS_GPIO_INUM)); // disable interrupts by external GPIOs

    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });

  ArduinoOTA.begin();
  Serial.println("OTA ready");
}

void function_ota_handle( ) {
  ArduinoOTA.handle();
}
