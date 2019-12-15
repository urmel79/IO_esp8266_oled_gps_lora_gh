#include <Arduino.h>

#include "config.hpp"

// include wifi functionality for asynchronous wifi handling
#include "function_wifi.hpp"

// include OTA functionality (ability for Over The Air updates)
#include "function_ota.hpp"

#include "function_oled.hpp"

#include "function_gps.hpp"

#include "function_mqtt.hpp"

// #include "function_lora_rh.hpp"
#include "function_lora_sm.hpp"

TinyGPSPlus g_s_gps_values;

bool g_b_wifi_connected = false;
bool g_b_iicOLED_connected = false;
// bool g_b_valid_gps_signal = false;
// unsigned long g_ul_delayTime = 2000; // ms

//variables for blinking an LED with Millis
// D3 = GPIO0 = external LED
// D4 = GPIO2 = internal Wifi LED
//int pin_led = BUILTIN_LED;
// const int g_i_led_pin = 2; // ESP8266 Pin to which onboard LED is connected
unsigned long g_ul_previousMillis = 0;  // will store last time LED was updated
const unsigned long g_ul_delayTime = 2000;  // interval at which to blink (milliseconds)
bool g_b_ledState = false;  // ledState used to set the LED

void setup() {
  // Serial.begin(9600);
  Serial.begin(57600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  initSS_gps();

  // pinMode(g_i_led_pin, OUTPUT);

  configureWifi();

  configureMqtt(); // setup MQTT functionality

  connectToWifi();

  connectMqttPubTasks();

  g_b_iicOLED_connected = connectOLEDiic();

  function_lora_setup();
}

void loop() {
  if (get_wifi_isConnected()) {
    function_ota_handle();  // call handler function for OTA
  }

  function_lora_send_handler();

  g_s_gps_values = read_gps();

  // loop to blink without delay
  unsigned long currentMillis = millis();
  if (currentMillis - g_ul_previousMillis >= g_ul_delayTime) {
    // save the last time you blinked the LED
    g_ul_previousMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
    g_b_ledState = not(g_b_ledState);
    // set the LED with the ledState of the variable:
    // digitalWrite(g_i_led_pin,  g_b_ledState);

    // updating the oled display non-blocking with Ticker does produce hard panic errors (esp8266 crashes)
    if (g_b_iicOLED_connected) {
      printOLED_begin();
      printOLED_values_str(0, "Host: ", get_wifi_hostname());

#if defined(LORA_OLED)
      String l_str_msgIDs = "MsgID: " + String(function_lora_get_msgID()) + ", int: " + String(function_lora_get_msgCount());
      printOLED_str(10, l_str_msgIDs);
      String l_str_rssi_snr = "RSSI: " + String(function_lora_get_rssi()) + ", SNR: " + String(function_lora_get_snr());
      printOLED_str(20, l_str_rssi_snr);
#else
      printOLED_values_str(10, "IP: ", get_wifi_IP_str());
      printOLED_values_flt(20, "RSSI: ", get_wifi_RSSI(), 2, 0);
#endif

      function_lora_serial_out();

      Serial.println(get_wifi_hostname());
      Serial.println(get_wifi_IP_str());
      Serial.println(get_wifi_RSSI());

      mqtt_set_wifi_rssi_dBm(get_wifi_RSSI());

      if (g_s_gps_values.location.isValid()) {
        Serial.print("Latitude  : ");
        Serial.print(g_s_gps_values.location.lat(), 9);
        Serial.print(char(176));                            // ° symbol
        Serial.println(" N");
        Serial.print("Longitude : ");
        Serial.print(g_s_gps_values.location.lng(), 9);
        Serial.print(char(176));                            // ° symbol
        Serial.println(" E");
        Serial.print("Satellites: ");
        Serial.println(g_s_gps_values.satellites.value());
        Serial.print("Elevation : ");
        Serial.print(g_s_gps_values.altitude.meters());
        Serial.println(" m");
        Serial.print("Time Raw  : ");
        Serial.println(g_s_gps_values.time.value());        // raw time
        Serial.print("Time UTC  : ");
        Serial.print(g_s_gps_values.time.hour());           // GPS time UTC
        Serial.print(":");
        Serial.print(g_s_gps_values.time.minute());         // Minutes
        Serial.print(":");
        Serial.println(g_s_gps_values.time.second());       // Seconds
        Serial.print("Heading   : ");
        Serial.print(g_s_gps_values.course.deg());
        Serial.println(char(176));                          // ° symbol
        Serial.print("Speed     : ");
        Serial.print(g_s_gps_values.speed.kmph());
        Serial.println(" km/h");

        printOLED_values_flt(30, "Lat.: ", g_s_gps_values.location.lat(), 12, 9);
        printOLED_values_flt(40, "Long.: ", g_s_gps_values.location.lng(), 12, 9);
        printOLED_values_flt(50, "Sat.: ", g_s_gps_values.satellites.value(), 5, 0);
        printOLED_end();

        String l_str_time = String(g_s_gps_values.date.year()) + "-"
                          + String(g_s_gps_values.date.month()) + "-"
                          + String(g_s_gps_values.date.day()) + " "
                          + String(g_s_gps_values.time.hour()) + ":"
                          + String(g_s_gps_values.time.minute())  + ":"
                          + String(g_s_gps_values.time.second());
        // String l_str_time = String(g_s_gps_values.time.value());

        mqtt_set_gps_valid(true);
        // mqtt_set_gps_json(String sensor, int time, String location, String icon, double latitude, double longitude);
        mqtt_set_gps_json("lora_1_gps", l_str_time, "mobile bug", "fa-bug", "Yellow",
                          g_s_gps_values.satellites.value(),
                          g_s_gps_values.altitude.meters(),
                          get_wifi_RSSI(),
                          g_s_gps_values.location.lat(),
                          g_s_gps_values.location.lng());
      }
      else {
        Serial.println("#### No GPS Signal .. ####");
        mqtt_set_gps_valid(false);

        printOLED_str(30, "# No GPS Signal .. #");
        printOLED_str(40, "");
        printOLED_str(50, "");
        printOLED_end();
      }
    }
  }
}






//
