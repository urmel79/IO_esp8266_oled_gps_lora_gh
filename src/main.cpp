#include <Arduino.h>

#ifdef WDT_ENABLE
  #ifdef ESP32
    #include <esp_task_wdt.h>

    // 3 seconds WDT
    #define WDT_TIMEOUT 3
  #endif
#endif

#include "config.hpp"

// include wifi functionality for asynchronous wifi handling
#include "function_wifi.hpp"

// include OTA functionality (ability for Over The Air updates)
#include "function_ota.hpp"

#include "function_oled.hpp"

#include "function_gps.hpp"

#include "function_mqtt.hpp"

#ifdef LORA_TOPO_P2P
  // #include "function_lora_rh.hpp"
  #include "function_lora_sm.hpp"
#elif LORA_TOPO_TTN
  #include "function_lorawan_ttn.hpp"
#endif

TinyGPSPlus g_s_gps_values;
struct_gps_RunningAVG_Median g_s_gps_avg;

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
  Serial.begin(115200);
  // while (!Serial) {
  //   ; // wait for serial port to connect. Needed for native USB port only
  // }

// enable watchdog functionality
#ifdef WDT_ENABLE
  #ifdef ESP32
    // Initialize the Task Watchdog Timer (TWDT)
    // Enable panic so ESP32 restarts
    // Parameters:
    //   timeout: timeout period of TWDT in seconds
    //   panic: flag that controls whether the panic handler will be executed when the TWDT times out
    esp_task_wdt_init(WDT_TIMEOUT, true);

    // Subscribe current thread (handle=NULL) to TWDT for watching
    // Each subscribed task must periodically call esp_task_wdt_reset() to prevent the TWDT from elapsing its timeout period.
    esp_task_wdt_add(NULL);
  #elif ESP8266
    ESP.wdtEnable(0); // how to set the desired timeout??
    // disable the software watchdog (hardware watchdog will bite after timeout of about 8 s ...)
    ESP.wdtDisable();
  #endif
#endif

  initSS_gps();

  // pinMode(g_i_led_pin, OUTPUT);

  function_wifi_setup();

  function_mqtt_setup(); // setup MQTT functionality

  function_wifi_connect();

  function_mqtt_connect_PubTasks();

  g_b_iicOLED_connected = connectOLEDiic();

#ifdef LORA_TOPO_P2P
  function_lora_setup();
#elif LORA_TOPO_TTN
  function_lorawan_ttn_setup();
#endif
}

void loop() {
// feed the watchdog so that hopefully it doesn't bite you
#ifdef WDT_ENABLE
  #ifdef ESP32
    // reset the TWDT on behalf of the currently running task
    esp_task_wdt_reset();
  #elif ESP8266
    ESP.wdtFeed();
  #endif
#endif

  if (get_wifi_isConnected()) {
    function_ota_handle();  // call handler function for OTA
  }

#ifdef LORA_TOPO_P2P
  function_lora_send_handler();
#elif LORA_TOPO_TTN
  function_lorawan_ttn_send_handler();
#endif

  g_s_gps_values = read_gps();

  // loop main task without delay
  unsigned long currentMillis = millis();
  if (currentMillis - g_ul_previousMillis >= g_ul_delayTime) {
    // save the last time the main task was called
    g_ul_previousMillis = currentMillis;
    // if the LED is off turn it on and vice-versa:
    g_b_ledState = not(g_b_ledState);
    // set the LED with the ledState of the variable:
    // digitalWrite(g_i_led_pin,  g_b_ledState);

    // updating the oled display non-blocking with Ticker does produce hard panic errors (esp8266 crashes)
    if (g_b_iicOLED_connected) {
      printOLED_begin();
      printOLED_values_str(0, "Host: ", get_wifi_hostname());

#if defined(LORA_OLED_P2P) && defined(LORA_TOPO_P2P)
      String l_str_msgIDs = "MsgID: " + String(function_lora_get_msgID()) + ", int: " + String(function_lora_get_msgCount());
      printOLED_str(10, l_str_msgIDs);
      String l_str_rssi_snr = "RSSI: " + String(function_lora_get_rssi()) + ", SNR: " + String(function_lora_get_snr());
      printOLED_str(20, l_str_rssi_snr);
#else
      printOLED_values_str(10, "IP: ", get_wifi_IP_str());
      printOLED_values_flt(20, "RSSI: ", get_wifi_RSSI(), 2, 0, " dBm");
#endif

#ifdef LORA_TOPO_P2P
      function_lora_serial_out();
#endif

      Serial.printf("[WIFI] Hostname: %s\r\n", get_wifi_hostname().c_str());
      Serial.printf("[WIFI] IP      : %s\r\n", get_wifi_IP_str().c_str());
      Serial.printf("[WIFI] RSSI    : %d dBm\r\n", get_wifi_RSSI());

      mqtt_set_wifi_rssi_dBm(get_wifi_RSSI());

      if (g_s_gps_values.location.isValid()) {
        // smoothing GPS positions: calculate running (floating) average and median
        gps_RunningAVG_Median_addValues();
        g_s_gps_avg = get_gps_RunningAVG_Median();

        // Serial.println("### GPS Start ###");
        Serial.printf("[GPS] Latitude  : %.9f* N\r\n", g_s_gps_values.location.lat());
        Serial.printf("[GPS] Longitude : %.9f* E\r\n", g_s_gps_values.location.lng());
        Serial.printf("[GPS] Lat (AVG) : %.9f* N\r\n", g_s_gps_avg.gps_RunningAVG_lat);
        Serial.printf("[GPS] Lng (AVG) : %.9f* E\r\n", g_s_gps_avg.gps_RunningAVG_lng);
        Serial.printf("[GPS] Elevation : %f m\r\n", g_s_gps_avg.gps_RunningAVG_alt);

        // Serial.printf("Lat (AVG) Buffer size: %d, Buffer count: %d\r\n", g_s_gps_avg.gps_RunningAVG_lat_size, g_s_gps_avg.gps_RunningAVG_lat_cnt);
        // Serial.printf("Lng (AVG) Buffer size: %d, Buffer count: %d\r\n", g_s_gps_avg.gps_RunningAVG_lng_size, g_s_gps_avg.gps_RunningAVG_lng_cnt);
        // Serial.printf("Alt (AVG) Buffer size: %d, Buffer count: %d\r\n", g_s_gps_avg.gps_RunningAVG_alt_size, g_s_gps_avg.gps_RunningAVG_alt_cnt);

        Serial.printf("[GPS] Satellites: %d\r\n", g_s_gps_values.satellites.value());
        Serial.printf("[GPS] Time Raw  : %d\r\n", g_s_gps_values.time.value()); // raw time
        Serial.printf("[GPS] Time UTC  : %d:%d:%d\r\n", g_s_gps_values.time.hour(), g_s_gps_values.time.minute(), g_s_gps_values.time.second()); // GPS time UTC
        Serial.printf("[GPS] Heading   : %.2f*\r\n", g_s_gps_values.course.deg());
        Serial.printf("[GPS] Speed     : %.2f km/h\r\n", g_s_gps_values.speed.kmph());
        // Serial.println("### GPS End ###");

        printOLED_values_flt(30, "Lat.: ", g_s_gps_values.location.lat(), 12, 9, "° N");
        printOLED_values_flt(40, "Lng.: ", g_s_gps_values.location.lng(), 12, 9, "° E");
        printOLED_values_flt(50, "Sat.: ", g_s_gps_values.satellites.value(), 2, 0, "");
        printOLED_end();

        String l_str_time = String(g_s_gps_values.date.year()) + "-"
                          + String(g_s_gps_values.date.month()) + "-"
                          + String(g_s_gps_values.date.day()) + " "
                          + String(g_s_gps_values.time.hour()) + ":"
                          + String(g_s_gps_values.time.minute())  + ":"
                          + String(g_s_gps_values.time.second());

        mqtt_set_gps_valid(true);

        mqtt_set_gps_json(MQTT_SENSOR_NODE, l_str_time, "mobile bug", "fa-bug", GPS_COLOR,
                          g_s_gps_values.satellites.value(),
                          g_s_gps_values.altitude.meters(),
                          get_wifi_RSSI(),
                          g_s_gps_values.location.lat(),
                          g_s_gps_values.location.lng(),
                          g_s_gps_avg.gps_RunningAVG_lat,
                          g_s_gps_avg.gps_RunningAVG_lng,
                          g_s_gps_avg.gps_RunningAVG_alt,
                          g_s_gps_avg.gps_RunningMedian_lat,
                          g_s_gps_avg.gps_RunningMedian_lng,
                          g_s_gps_avg.gps_RunningMedian_alt);

#if LORA_TOPO_TTN
        function_lorawan_ttn_send_gps(g_s_gps_avg.gps_RunningAVG_lat, g_s_gps_avg.gps_RunningAVG_lng);
#endif
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
