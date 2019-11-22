#include "function_lora_rh.hpp"

#include <SPI.h>

// install path: ~/.platformio/lib/RadioHead/
#include <RH_RF95.h>
#include <RHReliableDatagram.h>

//Radio pinout setup
#define RFM95_CS  15 // CS pin is connected to NodeMCU GPIO15
#define RFM95_RST 16 // RST pin is connected to NodeMCU GPIO16
#define RFM95_INT 10 // G0 pin is connected to NodeMCU GPIO10

#define RF95_FREQ 868.0

// Bw125Cr45Sf128:   Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Default medium range.
// Bw500Cr45Sf128:   Bw = 500 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on. Fast+short range.
// Bw31_25Cr48Sf512: Bw = 31.25 kHz, Cr = 4/8, Sf = 512chips/symbol, CRC on. Slow+long range.
// Bw125Cr48Sf4096:  Bw = 125 kHz, Cr = 4/8, Sf = 4096chips/symbol, CRC on. Slow+long range.
#define RF95_MODEM_CONFIG RH_RF95::Bw125Cr45Sf128

#define RFM95_TIMEOUT 4000
#define RFM95_MNG_TIMEOUT 500

#define RFM95_CLIENT_ADDRESS 1
#define RFM95_SERVER_ADDRESS 2

// Singleton instance of the radio driver
// web doc: http://www.airspayce.com/mikem/arduino/RadioHead/classRH__RF95.html
//RH_RF95 rf95;
//RH_RF95 rf95(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W
//RH_RF95 rf95(8, 3); // Adafruit Feather M0 with RFM95
RH_RF95 rf95_driver(RFM95_CS, RFM95_INT);

// // Class to manage message delivery and receipt, using the driver declared above
// // web doc: http://www.airspayce.com/mikem/arduino/RadioHead/classRHReliableDatagram.html
// #if defined(LORA_SENDER)
//   RHReliableDatagram rf95_manager(rf95_driver, RFM95_CLIENT_ADDRESS);
// #elif defined(LORA_RECEIVER)
//   RHReliableDatagram rf95_manager(rf95_driver, RFM95_SERVER_ADDRESS);
// #endif

// Need this on Arduino Zero with SerialUSB port (eg RocketScream Mini Ultra Pro)
//#define Serial SerialUSB

#define LED_PIN    2  // internal Wifi LED of NodeMCU (negative logic)
// #define LED_PIN    LED_BUILTIN  // build-in LED of NodeMCU (negative logic)

// Ticker rf95_manager_send_ticker;
// Ticker rf95_manager_receive_ticker;

void function_lora_setup( void ) {
  // Rocket Scream Mini Ultra Pro with the RFM95W only:
  // Ensure serial flash is not interfering with radio communication on SPI bus
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  pinMode(LED_PIN, OUTPUT); // internal Wifi LED

  if (!rf95_driver.init()) Serial.println("LoRa: init failed");

  // reset LoRa module
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  //  driver.setTxPower(23, false);
  // If you are using Modtronix inAir4 or inAir9, or any other module which uses the
  // transmitter RFO pins and not the PA_BOOST pins
  // then you can configure the power transmitter power for -1 to 14 dBm and with useRFO true.
  // Failure to do that will result in extremely low transmit powers.
  //  driver.setTxPower(14, true);

  // while (!rf95_manager.init()) {
  while (!rf95_driver.init()) {
#if defined(LORA_SENDER)
    Serial.println("LoRa: initializing client node (ReliableDatagram manager)");
#elif defined(LORA_RECEIVER)
    Serial.println("LoRa: initializing server node (ReliableDatagram manager)");
#endif
    while (1);
  }

#if defined(LORA_SENDER)
  Serial.println("LoRa: initialisation of client succeeded (ReliableDatagram manager)");
#elif defined(LORA_RECEIVER)
  Serial.println("LoRa: initialisation of server succeeded (ReliableDatagram manager)");
#endif

  if (!rf95_driver.setFrequency(RF95_FREQ)) {
    Serial.println("LoRa: set frequency failed");
    while (1);
  }
  Serial.print("LoRa: set Freq to: ");
  Serial.print(RF95_FREQ);
  Serial.println(" MHz");
  rf95_driver.setTxPower(23, false);

  if (!rf95_driver.setModemConfig(RF95_MODEM_CONFIG)) {
    Serial.println("LoRa: setModemConfig failed");
    while (1);
  }
  Serial.println("LoRa: setModemConfig succeeded");

  // rf95_manager.setTimeout(RFM95_MNG_TIMEOUT);

  // // start sender ticker
  // rf95_manager_send_ticker.attach(2, function_lora_sendtoWait);
}

#if defined(LORA_SENDER)
  uint8_t data[] = "Hello World!";
#elif defined(LORA_RECEIVER)
  uint8_t data[] = "And hello back to you";
#endif

// Dont put this on the stack:
uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];

void function_lora_sendtoWait( void ) {
  // if (rf95_manager.available()) {
  // if (rf95_driver.available()) {
    Serial.println("LoRa: sending to rf95_reliable_datagram_server");

    digitalWrite(LED_PIN, LOW); // negative logic

    // Send a message to rf95_reliable_datagram_server
    // if (rf95_manager.sendtoWait(data, sizeof(data), RFM95_SERVER_ADDRESS)) {
    if (rf95_driver.send(data, sizeof(data))) {
      // start ticker for polling the answer
      // rf95_manager_receive_ticker.attach(0.5, rf95_manager_receive);

      rf95_driver.waitPacketSent();
      Serial.println("LoRa: sendtoWait succeeded");
      // return true;
    }
    else {
      Serial.println("LoRa: sendtoWait failed");
      // return false;
    }
  // }
  // else {
  //   Serial.println("LoRa: rf95_manager is not available");
  //   // return false;
  // }
}

// void rf95_manager_receive( void ) {
//   if (rf95_manager.available()) {
//     // Now wait for a reply from the server
//     uint8_t len = sizeof(buf);
//     uint8_t from;
//     if (rf95_manager.recvfromAckTimeout(buf, &len, RFM95_TIMEOUT, &from)) {
//       Serial.print("LoRa: got reply from: 0x");
//       Serial.print(from, HEX);
//       Serial.print(": ");
//       Serial.println((char*)buf);
//       Serial.print("LoRa: RSSI: ");
//       Serial.println(rf95_driver.lastRssi(), DEC);
//
//       digitalWrite(LED_PIN, HIGH); // negative logic
//
//       // stop ticker for polling the answer
//       rf95_manager_receive_ticker.detach();
//     }
//     else {
//       Serial.println("LoRa: no reply, is rf95_reliable_datagram_server running?");
//     }
//   }
//   else {
//     Serial.println("LoRa: rf95_manager is not available");
//   }
// }

























//
