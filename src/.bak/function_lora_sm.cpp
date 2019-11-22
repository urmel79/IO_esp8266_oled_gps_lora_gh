#include "function_lora_sm.hpp"

#include <SPI.h>

// install path: ~/.platformio/lib/LoRa_ID1167/
// https://github.com/sandeepmistry/arduino-LoRa
#include <LoRa.h>

//Radio pinout setup
#define RFM95_CS  15 // CS pin is connected to NodeMCU GPIO15
#define RFM95_RST 16 // RST pin is connected to NodeMCU GPIO16
#define RFM95_INT 10 // G0 pin is connected to NodeMCU GPIO10

// #define RF95_FREQ 868.0
#define RF95_FREQ       868E6
#define RFM95_TX_POWER  20

#define RFM_TX_INTERVAL 2000

// #define RFM95_TIMEOUT 4000
// #define RFM95_MNG_TIMEOUT 500

// #define RFM95_CLIENT_ADDRESS 1
// #define RFM95_SERVER_ADDRESS 2

#define LED_PIN    2  // internal Wifi LED of NodeMCU (negative logic)
// #define LED_PIN    LED_BUILTIN  // build-in LED of NodeMCU (negative logic)

// Ticker rf95_manager_send_ticker;
// Ticker rf95_manager_receive_ticker;

unsigned long g_ul_previousMillis_lora = 0;  // will store last time LED was updated

void function_lora_setup( void ) {
  // Rocket Scream Mini Ultra Pro with the RFM95W only:
  // Ensure serial flash is not interfering with radio communication on SPI bus
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  pinMode(LED_PIN, OUTPUT); // internal Wifi LED

  // reset LoRa module
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  // override the default CS, reset, and IRQ pins (optional)
  LoRa.setPins(RFM95_CS, RFM95_RST, RFM95_INT);  // set CS, reset, IRQ pin

  if (!LoRa.begin(RF95_FREQ)) {  // initialize radio at 868 MHz
#if defined(LORA_SENDER)
    Serial.println("LoRa: initializing client node");
#elif defined(LORA_RECEIVER)
    Serial.println("LoRa: initializing server node");
#endif
    while (1);
  }

#if defined(LORA_SENDER)
  Serial.println("LoRa: initialisation of client succeeded");
#elif defined(LORA_RECEIVER)
  Serial.println("LoRa: initialisation of server succeeded");
#endif

  Serial.print("LoRa: set tx power to: ");
  Serial.print(RFM95_TX_POWER);
  Serial.println(" dB");
  LoRa.setTxPower(RFM95_TX_POWER, PA_OUTPUT_PA_BOOST_PIN);

  LoRa.onReceive(onReceive);
  LoRa_rxMode();

  // // start sender ticker
  // rf95_manager_send_ticker.attach(2, function_lora_send);
}

void LoRa_rxMode( void ) {
  LoRa.enableInvertIQ();                // active invert I and Q signals
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode( void ) {
  LoRa.idle();                          // set standby mode
  LoRa.disableInvertIQ();               // normal mode
}

void LoRa_sendMessage( String message ) {
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.print(message);                  // add payload
  LoRa.endPacket();                     // finish packet and send it
  LoRa_rxMode();                        // set rx mode
}

void function_lora_send_continuosly( void ) {
  // Serial.println("### DEBUG: was here .. ###");

  // repeat every 2000 milli seconds
  if (runEvery(RFM_TX_INTERVAL)) {
    digitalWrite(LED_PIN, LOW); // negative logic

    String message = "HeLoRa World! ";
    message += "I'm a Node! ";
    message += millis();
    LoRa_sendMessage(message); // send a message

    Serial.print("LoRa: sent message '");
    Serial.print(message);
    Serial.println("'");
  }
  digitalWrite(LED_PIN, HIGH); // negative logic
}

void onReceive(int packetSize) {
  String message = "";

  while (LoRa.available()) {
    message += (char)LoRa.read();
  }

  Serial.print("LoRa: node received message: '");
  Serial.print(message);
  Serial.println("'");

  Serial.println("LoRa: RSSI: " + String(LoRa.packetRssi()));
  Serial.println("LoRa: SNR: " + String(LoRa.packetSnr()));
}

bool runEvery(unsigned long interval) {
  // static unsigned long previousMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - g_ul_previousMillis_lora >= interval) {
    // save the last time you blinked the LED
    g_ul_previousMillis_lora = currentMillis;

    Serial.println("### DEBUG: was here .. ###");

    return true;
  }
  return false;
}
























//
