#include "function_wifi.hpp"

//
// Inspiration for asynchronous Wifi handling and it's differences between ESP8266 and ESP32 was found at:
//
// https://github.com/marvinroger/async-mqtt-client/blob/master/examples/FullyFeatured-ESP8266/FullyFeatured-ESP8266.ino
// https://github.com/marvinroger/async-mqtt-client/blob/master/examples/FullyFeatured-ESP32/FullyFeatured-ESP32.ino
//

#include "function_mqtt.hpp"
#include "function_ota.hpp"
#include "function_gps.hpp"

// 'ESP32' and 'ESP8266' are defined within PlatformIO
#ifdef ESP32
  // #include <ets_sys_bk.hpp>
  // #define ETS_GPIO_INUM       12
#elif ESP8266
  WiFiEventHandler wifiConnectHandler;
  WiFiEventHandler wifiDisconnectHandler;
#endif

Ticker wifiReconnectTimer;

void configureWifi() {
  // 'ESP32' and 'ESP8266' are defined within PlatformIO
  #ifdef ESP32
    WiFi.onEvent(WiFiEvent);
  #elif ESP8266
    wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  #endif

  WiFi.softAPdisconnect(true); // deactivate wifi access point (soft AP)
}

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.mode(WIFI_STA);  // client mode

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

#ifdef ESP32
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE); // call is only a workaround for bug in WiFi class
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  // Alternative 1:
  // [Bug:] hostname cannot be set in the moment => bug in Wifi class?
  //  issue: https://github.com/espressif/arduino-esp32/issues/806
  //  workaround: call 'WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);' before 'WiFi.begin(..)'
  //              suggestion found here: https://github.com/espressif/arduino-esp32/issues/2537
  WiFi.setHostname(HOSTNAME); // set hostname

#elif ESP8266
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  WiFi.hostname(String(HOSTNAME)); // set hostname
#endif

  // enable external interrupts again
  // ETS_GPIO_INTR_ENABLE();
  // ets_isr_unmask((1<<ETS_GPIO_INUM)); // enable interrupts by external GPIOs
  // Enable interrupts on the rx pin after wifi (re)connect
  function_gps_enable_Rx();
}

bool get_wifi_isConnected() {
  return WiFi.isConnected();
}

// 'ESP32' and 'ESP8266' are defined within PlatformIO
#ifdef ESP32
void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) {
  case SYSTEM_EVENT_STA_START:
    // Alternative 2:
    // set sta hostname here
    //
    // Hint from:
    // https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiIPv6/WiFiIPv6.ino#L79
    //
    // WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE); // call is only a workaround for bug in WiFi class
    // WiFi.setHostname(HOSTNAME);
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    Serial.println("Connected to Wi-Fi.");

    Serial.print("Wifi AP (SSID): ");
    Serial.println(String(WiFi.SSID()));
    Serial.print("ESP32 hostname: ");
    Serial.println(String(WiFi.getHostname()));
    Serial.print("ESP32 MAC: ");
    Serial.println(WiFi.macAddress());
    Serial.print("ESP32 IP: ");
    Serial.println(WiFi.localIP());
    Serial.print("Wi-Fi signal level: ");
    Serial.println(WiFi.RSSI());

    function_ota_setup(HOSTNAME);   // setup OTA functionality

    connectToMqtt();
    connectMqttPubTasks();
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Serial.println("Disconnected from Wi-Fi.");
    deactivateMqtt_reconnectTimer();
    wifiReconnectTimer.once(2, connectToWifi);
    break;
  }
}
#elif ESP8266
void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");

  Serial.print("Wifi AP (SSID): ");
  Serial.println(String(WiFi.SSID()));
  Serial.print("ESP8266 hostname: ");
  Serial.println(String(WiFi.hostname()));
  Serial.print("ESP8266 MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("ESP8266 IP: ");
  Serial.println(WiFi.localIP());
  Serial.print("Wi-Fi signal level: ");
  Serial.println(WiFi.RSSI());

  function_ota_setup(HOSTNAME);   // setup OTA functionality

  connectToMqtt();
  connectMqttPubTasks();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  deactivateMqtt_reconnectTimer();
  wifiReconnectTimer.once(2, connectToWifi);
}
#endif

IPAddress get_wifi_IP() {
  return WiFi.localIP();
}

String get_wifi_IP_str() {
  return WiFi.localIP().toString().c_str();
}

String get_wifi_hostname() {
  #ifdef ESP32
    return WiFi.getHostname();
  #elif ESP8266
    return WiFi.hostname();
  #endif
}

int get_wifi_RSSI() {
  return WiFi.RSSI();
}
