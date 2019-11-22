#ifndef function_lora_sm_hpp
#define function_lora_sm_hpp

#include "config.hpp"

#include <Arduino.h>

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




















#endif // function_lora_sm_hpp
