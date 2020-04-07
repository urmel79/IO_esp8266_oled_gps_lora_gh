#ifndef function_lora_sm_hpp
#define function_lora_sm_hpp

#include "config.hpp"

#include <Arduino.h>

#if defined(NODE_MCU_LORA_1) || defined(NODE_MCU_LORA_2)
  //Radio pinout setup
  #define RFM95_CS  15  // CS pin is connected to NodeMCU GPIO15
  #define RFM95_RST 16  // RST pin is connected to NodeMCU GPIO16
  #define RFM95_INT 10  // C0 pin is connected to NodeMCU GPIO10

  #define LED_PIN   2   // internal Wifi LED of NodeMCU (negative logic)
#elif defined(NODE_MCU_LORA_2_ESP32)
  //Radio pinout setup
  #define RFM95_CS  5   // CS pin is connected to NodeMCU GPIO15
  #define RFM95_RST 4   // RST pin is connected to NodeMCU GPIO16
  #define RFM95_INT 15  // C0 pin is connected to NodeMCU GPIO10

  #define LED_PIN   2   // GPIO2 pin of builtin LED at NodeMCU ESP-32S (negative logic)
  // #define LED_PIN   33  // most ESP32 Boards don't have builtin LEDs, so an external LED is connected to GPIO33 (with 220 Ohm pullup resistor)
#endif

#define RF95_FREQ       868E6 // frequency 868 MHz, channel 0
// #define RF95_FREQ       868.0E6 // frequency 868 MHz, channel 0
// #define RF95_FREQ       868.1E6 // frequency 868 MHz, channel 1
#define RFM95_TX_POWER  17      // transmitting power [dB]; max. 23 dB
#define RFM95_SF        7       // spreading factor

void function_lora_setup( void );

void LoRa_rxMode( void );

void LoRa_txMode( void );

void LoRa_sendMessage(String message);

void function_lora_send_handler( void );

void onReceive(int packetSize);

bool runEvery(unsigned long interval);

int function_lora_get_rssi( void );
float function_lora_get_snr( void );
int function_lora_get_msgID( void );
int function_lora_get_msgCount( void );

void function_lora_serial_out( void );




















#endif // function_lora_sm_hpp
