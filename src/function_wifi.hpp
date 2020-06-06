#ifndef function_wifi_hpp
#define function_wifi_hpp

// 'ESP32' and 'ESP8266' are defined within PlatformIO
#ifdef ESP32
  #warning "ESP32 was defined: doing ESP32 stuff"
  #include <WiFi.h>
#elif ESP8266
  #warning "ESP8266 was defined: doing ESP8266 stuff"
  #include <ESP8266WiFi.h>
#else
  #error "Neither ESP32 nor ESP8266 was defined - Pls help me :("
#endif

#include <Ticker.h>

#include "config.hpp"

void function_wifi_setup();
void function_wifi_connect();
void function_wifi_reconnect();

// 'ESP32' and 'ESP8266' are defined within PlatformIO
#ifdef ESP32
  void WiFiEvent(WiFiEvent_t event);
  void function_wifiEvent_enable( void );
  void function_wifiEvent_disable( void );
#elif ESP8266
  void onWifiConnect(const WiFiEventStationModeGotIP& event);
  void onWifiDisconnect(const WiFiEventStationModeDisconnected& event);
#endif

IPAddress get_wifi_IP();
String get_wifi_IP_str();
String get_wifi_hostname();
int get_wifi_RSSI();
bool get_wifi_isConnected();

#endif // function_wifi_hpp
