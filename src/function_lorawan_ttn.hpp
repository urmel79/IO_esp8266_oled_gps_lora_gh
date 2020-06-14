#ifndef function_lorawan_ttn_hpp
#define function_lorawan_ttn_hpp

#ifdef LORA_TOPO_TTN  // build only when it is really needed

  #include "config.hpp"

  #if defined(NODE_MCU_LORA_2_ESP32)
    //Radio pinout setup
    #define RFM95_CS  5    // CS pin is connected to NodeMCU ESP-32S GPIO5
    #define RFM95_RST 4    // RST pin is connected to NodeMCU ESP-32S GPIO4
    #define RFM95_INT0 15  // C0 pin is connected to NodeMCU ESP-32S GPIO15
    #define RFM95_INT1 27  // C1 pin is connected to NodeMCU ESP-32S GPIO27

    // #define LED_PIN   2     // GPIO2 pin of builtin LED at NodeMCU ESP-32S (negative logic)
    #define LED_PIN   33  // most ESP32 Boards don't have builtin LEDs, so an external LED is connected to GPIO33 (with 220 Ohm pullup resistor)

    // Digital pin connected to the DHT sensor
    #define DHTPIN 32 // GPIO32
    #define DHTTYPE DHT11
  #endif

  #include <lmic.h>     // LMIC-Arduino by IBM: https://github.com/matthijskooijman/arduino-lmic

  #include <Arduino.h>

  void onEvent(ev_t ev);
  void do_send(osjob_t* j);

  void function_lorawan_ttn_setup( void );

  void function_lorawan_ttn_send_handler( void );

  void function_lorawan_ttn_send_gps(float lat_avg, float lng_avg);

  void function_LoRaEvent_disable( void );
  void function_LoRaEvent_enable( void );

  void function_lora_reactivate_Tx( void );

  String function_convert_uptime2string(unsigned long time_s);








#endif // LORA_TOPO_TTN



#endif // function_lorawan_ttn_hpp



//
