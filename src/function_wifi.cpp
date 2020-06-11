#include "function_wifi.hpp"

//
// Inspiration for asynchronous Wifi handling and it's differences between ESP8266 and ESP32 was found at:
//
// https://github.com/marvinroger/async-mqtt-client/blob/master/examples/FullyFeatured-ESP8266/FullyFeatured-ESP8266.ino
// https://github.com/marvinroger/async-mqtt-client/blob/master/examples/FullyFeatured-ESP32/FullyFeatured-ESP32.ino
//
// Inspiration for WiFi handling on the basis of ESP32 was found here:
// https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiClientEvents/WiFiClientEvents.ino
//

#include "function_mqtt.hpp"
#include "function_ota.hpp"
#include "function_gps.hpp"

// 'ESP32' and 'ESP8266' are defined within PlatformIO
#ifdef ESP32
  // #include <ets_sys_bk.hpp>
  // #define ETS_GPIO_INUM       12
  bool g_b_WifiEvent_enabled = false;
#elif ESP8266
  WiFiEventHandler wifiConnectHandler;
  WiFiEventHandler wifiDisconnectHandler;
#endif

Ticker wifiReconnectTimer;

void function_wifi_setup() {
  // 'ESP32' and 'ESP8266' are defined within PlatformIO
  #ifdef ESP32
    // enable wifi event handling
    g_b_WifiEvent_enabled = true;
    WiFi.onEvent(WiFiEvent);
  #elif ESP8266
    wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
    wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);
  #endif

  WiFi.softAPdisconnect(true); // deactivate wifi access point (soft AP)
}

void function_wifi_connect() {
  Serial.println("[WIFI] Connecting to Wi-Fi...");

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

  WiFi.mode(WIFI_STA);  // client mode

#ifdef ESP32
  // call is only a workaround for bug in WiFi class
  // ... maybe not necessary in current version of WiFi class <= NO!
  WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);

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

  // Enable interrupts on the rx pin after wifi (re)connect attempt
  function_gps_enable_Rx();
#endif

}

void function_wifi_reconnect() {
  Serial.println("[WIFI] Trying to reconnect to Wi-Fi...");

  function_gps_disable_Rx();
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

#if ESP8266
  // Enable interrupts on the rx pin after wifi (re)connect
  function_gps_enable_Rx();
#endif
}

bool get_wifi_isConnected() {
  return WiFi.isConnected();
}

// 'ESP32' and 'ESP8266' are defined within PlatformIO
#ifdef ESP32
// the function WiFiEvent() is called in a separate cyclic task automagically
void WiFiEvent(WiFiEvent_t event) {
  // if deactivated, return asap to prevent interrupt conflicts
  if (!g_b_WifiEvent_enabled) {
    return;
  }

  // Rx interrupts are conflicting with OTA updates => disable serial Rx!
  function_gps_disable_Rx();

  Serial.printf("[WIFI] event: %d - ", event);
  switch(event) {
  case SYSTEM_EVENT_WIFI_READY:   // event 0
      Serial.println("WiFi interface ready");
      // // Rx interrupts are conflicting with OTA updates => disable serial Rx!
      // function_gps_disable_Rx();
      break;
  case SYSTEM_EVENT_SCAN_DONE:   // event 1
      Serial.println("Completed scan for access points");
      break;
  case SYSTEM_EVENT_STA_START:   // event 2
      Serial.println("WiFi client started");
      // [fix (better)] take care of the cause of the problem: disable the sources of interrupts
      // here: serial communication to the gps sensor causes interrupts, when receiving new data
      // Rx interrupts are conflicting with OTA updates => disable serial Rx!
      // function_gps_disable_Rx();
      // function_LoRaEvent_disable();

      // call is only a workaround for bug in WiFi class
      // ... maybe not necessary in current version of WiFi class <== NO!
      WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);

      // Alternative 2:
      // set sta hostname here
      //
      // Hint from:
      // https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFi/examples/WiFiIPv6/WiFiIPv6.ino#L79
      //
      WiFi.setHostname(HOSTNAME);

      break;
  case SYSTEM_EVENT_STA_STOP:   // event 3
      Serial.println("WiFi clients stopped");

      // Enable interrupts on the rx pin after wifi (re)connect attempt
      function_gps_enable_Rx();

      break;
  case SYSTEM_EVENT_STA_CONNECTED:   // event 4
      Serial.println("Connected to access point");
      break;
  case SYSTEM_EVENT_STA_DISCONNECTED:   // event 5
      Serial.println("Disconnected from WiFi access point.");
      deactivateMqtt_reconnectTimer();

      // prevent automatic reconnect attempts
      WiFi.disconnect(true);

      // // the function WiFiEvent() is called in a separate cyclic task automagically;
      // // so you will get a very strange behaviour with the reconnecting ticker call ...
      // // ... it seems, the ESP32 hangs in an endless loop
      wifiReconnectTimer.attach(5, function_wifi_reconnect);
      break;
  case SYSTEM_EVENT_STA_AUTHMODE_CHANGE:   // event 6
      Serial.println("Authentication mode of access point has changed");
      break;
  case SYSTEM_EVENT_STA_GOT_IP:   // event 7
      Serial.println("Obtained IP address.");

      // deactivate reconnecting attempts
      wifiReconnectTimer.detach();

      // Serial.print("[WIFI] WiFi AP (SSID): ");
      // Serial.println(String(WiFi.SSID()));
      // Serial.print("[WIFI] ESP32 hostname: ");
      // Serial.println(String(WiFi.getHostname()));
      // Serial.print("[WIFI] ESP32 MAC: ");
      // Serial.println(WiFi.macAddress());
      // Serial.print("[WIFI] ESP32 IP: ");
      // Serial.println(WiFi.localIP());
      // Serial.print("[WIFI] WiFi signal level: ");
      // Serial.println(WiFi.RSSI());

      Serial.printf("[WIFI] AP SSID : %s\r\n", WiFi.SSID().c_str());
      Serial.printf("[WIFI] Hostname: %s\r\n", WiFi.getHostname());
      Serial.printf("[WIFI] MAC     : %s\r\n", WiFi.macAddress().c_str());
      Serial.printf("[WIFI] IP      : %s\r\n", WiFi.localIP().toString().c_str());
      Serial.printf("[WIFI] RSSI    : %d dBm\r\n", WiFi.RSSI());

      function_ota_setup(HOSTNAME);   // setup OTA functionality

      function_mqtt_connect();
      function_mqtt_connect_PubTasks();

      // // Enable interrupts on the rx pin after wifi (re)connect attempt
      // function_gps_enable_Rx();
      break;
  case SYSTEM_EVENT_STA_LOST_IP:   // event 8
      Serial.println("Lost IP address and IP address is reset to 0");
      break;
  case SYSTEM_EVENT_STA_WPS_ER_SUCCESS:   // event 9
      Serial.println("WiFi Protected Setup (WPS): succeeded in enrollee mode");
      break;
  case SYSTEM_EVENT_STA_WPS_ER_FAILED:   // event 10
      Serial.println("WiFi Protected Setup (WPS): failed in enrollee mode");
      break;
  case SYSTEM_EVENT_STA_WPS_ER_TIMEOUT:   // event 11
      Serial.println("WiFi Protected Setup (WPS): timeout in enrollee mode");
      break;
  case SYSTEM_EVENT_STA_WPS_ER_PIN:   // event 12
      Serial.println("WiFi Protected Setup (WPS): pin code in enrollee mode");
      break;
  case SYSTEM_EVENT_AP_START:   // event 13
      Serial.println("WiFi access point started");
      break;
  case SYSTEM_EVENT_AP_STOP:   // event 14
      Serial.println("WiFi access point  stopped");
      break;
  case SYSTEM_EVENT_AP_STACONNECTED:   // event 15
      Serial.println("Client connected");
      break;
  case SYSTEM_EVENT_AP_STADISCONNECTED:   // event 16
      Serial.println("Client disconnected");
      break;
  case SYSTEM_EVENT_AP_STAIPASSIGNED:   // event 17
      Serial.println("Assigned IP address to client");
      break;
  case SYSTEM_EVENT_AP_PROBEREQRECVED:   // event 18
      Serial.println("Received probe request");
      break;
  case SYSTEM_EVENT_GOT_IP6:   // event 19
      Serial.println("IPv6 is preferred");
      break;
  case SYSTEM_EVENT_ETH_START:   // event 20
      Serial.println("Ethernet started");
      break;
  case SYSTEM_EVENT_ETH_STOP:   // event 21
      Serial.println("Ethernet stopped");
      break;
  case SYSTEM_EVENT_ETH_CONNECTED:   // event 22
      Serial.println("Ethernet connected");
      break;
  case SYSTEM_EVENT_ETH_DISCONNECTED:   // event 23
      Serial.println("Ethernet disconnected");
      break;
  case SYSTEM_EVENT_ETH_GOT_IP:   // event 24
      Serial.println("Obtained IP address");
      break;
  default: break;
  }
}
#elif ESP8266
void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("[WIFI] Connected to Wi-Fi.");

  // Serial.print("[WIFI] WiFi AP (SSID): ");
  // Serial.println(String(WiFi.SSID()));
  // Serial.print("[WIFI] ESP8266 hostname: ");
  // Serial.println(String(WiFi.hostname()));
  // Serial.print("[WIFI] ESP8266 MAC: ");
  // Serial.println(WiFi.macAddress());
  // Serial.print("[WIFI] ESP8266 IP: ");
  // Serial.println(WiFi.localIP());
  // Serial.print("[WIFI] WiFi signal level: ");
  // Serial.println(WiFi.RSSI());

  Serial.printf("[WIFI] AP SSID : %s\r\n", WiFi.SSID().c_str());
  Serial.printf("[WIFI] Hostname: %s\r\n", WiFi.hostname().c_str());
  Serial.printf("[WIFI] MAC     : %s\r\n", WiFi.macAddress().c_str());
  Serial.printf("[WIFI] IP      : %s\r\n", WiFi.localIP().toString().c_str());
  Serial.printf("[WIFI] RSSI    : %d dBm\r\n", WiFi.RSSI());

  function_ota_setup(HOSTNAME);   // setup OTA functionality

  function_mqtt_connect();
  function_mqtt_connect_PubTasks();

  // Enable gps interrupts on the rx pin after wifi (re)connect
  function_gps_enable_Rx();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("[WIFI] Disconnected from WiFi.");
  deactivateMqtt_reconnectTimer();
  wifiReconnectTimer.once(5, function_wifi_reconnect);
}
#endif

IPAddress get_wifi_IP() {
  if (WiFi.status() == WL_CONNECTED) {
    return WiFi.localIP();
  }
  else {
    IPAddress fake_ip = {0,0,0,0};
    return fake_ip;
  }
}

String get_wifi_IP_str() {
  if (WiFi.status() == WL_CONNECTED) {
    return WiFi.localIP().toString().c_str();
  }
  else {
    return "0.0.0.0";
  }
}

String get_wifi_hostname() {
  if (WiFi.status() == WL_CONNECTED) {
  #ifdef ESP32
    return WiFi.getHostname();
  #elif ESP8266
    return WiFi.hostname();
  #endif
  }
  else return "# not connected #";
}

int get_wifi_RSSI() {
  if (WiFi.status() == WL_CONNECTED) {
    return WiFi.RSSI();
  }
  else return 0;
}

#ifdef ESP32
void function_wifiEvent_enable( void ) {
  g_b_WifiEvent_enabled = true;
}

void function_wifiEvent_disable( void ) {
  g_b_WifiEvent_enabled = false;
}
#endif



//
