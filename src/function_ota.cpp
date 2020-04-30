#include "function_ota.hpp"

#include "function_gps.hpp"

void function_ota_setup( const char *chr_hostname ) {
  // Port defaults to 8266
  ArduinoOTA.setPort(OTA_ESP_PORT);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(chr_hostname);

  // No authentication by default
  ArduinoOTA.setPassword(OTA_PASSWORD);

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    // [bug] external interrupts conflict with wifi connection attempts and
    // OTA updates and lead to sporadic crashes!
    // [fix (workaround)] disable all(!) external interrupts temporarily
    // reference https://www.mikrocontroller.net/topic/460256#5573848
    // ETS_GPIO_INTR_DISABLE();
    // ets_isr_mask((1<<ETS_GPIO_INUM)); // disable interrupts by external GPIOs
    // ESP_INTR_DISABLE(1<<ETS_GPIO_INUM);

    // [fix (better)] take care of the cause of the problem: disable the sources of interrupts
    // here: serial communication to the gps sensor causes interrupts, when receiving new data
    // Rx interrupts are conflicting with OTA updates => disable serial Rx!
    function_gps_disable_Rx();

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
    // enable external interrupts again => unnecessary, because mcu reboots ...
    // ETS_GPIO_INTR_ENABLE();
    // ets_isr_unmask((1<<ETS_GPIO_INUM)); // enable interrupts by external GPIOs
    // ESP_INTR_ENABLE(1<<ETS_GPIO_INUM);

    Serial.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    // disable external interrupts (they cause sporadic crashes!)
    // reference https://www.mikrocontroller.net/topic/460256#5573848
    // ETS_GPIO_INTR_DISABLE();
    // ets_isr_mask((1<<ETS_GPIO_INUM)); // disable interrupts by external GPIOs
    // ESP_INTR_DISABLE(1<<ETS_GPIO_INUM);

    // Rx interrupts are conflicting with OTA updates => disable serial Rx!
    function_gps_disable_Rx();

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
