#ifndef function_lora_sm_hpp
#define function_lora_sm_hpp

#include "config.hpp"

#include <Ticker.h>

#include <Arduino.h>

void function_lora_setup( void );

void LoRa_rxMode( void );

void LoRa_txMode( void );

void LoRa_sendMessage(String message);

void function_lora_send_continuosly( void );

void onReceive(int packetSize);

bool runEvery(unsigned long interval);

// void rf95_manager_receive( void );






















#endif // function_lora_sm_hpp
