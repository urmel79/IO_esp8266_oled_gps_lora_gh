#include "function_lora_sm.hpp"

#include <SPI.h>

// install path: ~/.platformio/lib/LoRa_ID1167/
// https://github.com/sandeepmistry/arduino-LoRa
#include <LoRa.h>

#if defined(LORA_SENDER)
  #define RFM_TX_INTERVAL 2000
  #define RFM_TX_TIMEOUT_CNT 5
  int g_i_rfm_tx_timeout_cnt_curr = 0;
  byte g_by_localAddress = 0xAA;        // address of this device
  byte g_by_destinationAddress = 0xBB;  // destination to send to
  byte g_by_broadcastAddress = 0xFF;    // broadcast address to send to
#elif defined(LORA_RECEIVER)
  #define RFM_TX_INTERVAL 300
  byte g_by_localAddress = 0xBB;        // address of this device
  byte g_by_destinationAddress = 0xAA;  // destination to send to
  byte g_by_broadcastAddress = 0xFF;    // broadcast address to send to
#endif

unsigned long g_ul_previousMillis_lora = 0;  // will store last time cycle was run

bool g_b_enable_send = false;
uint16_t g_i_msgCount = 0;       // count of outgoing messages
uint16_t g_i_incomingMsgId = 0;  // count of incoming messages
int g_i_rssi = 0;           // LoRa RSSI
float g_f_snr = 0;          // LoRa signal to noise ratio
String g_str_incoming_msg = "";
byte g_by_incomingLength = 0;
byte g_by_incomingRecipient = 0;
byte g_by_incomingSender = 0;

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
  LoRa.setPins(RFM95_CS, RFM95_RST, RFM95_INT);  // set CS, Reset, IRQ pin

  // initialize radio at 868 MHz
  if (!LoRa.begin(RF95_FREQ)) {
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

  Serial.print("LoRa: set spreading factor to: ");
  Serial.println(RFM95_SF);
  LoRa.setSpreadingFactor(RFM95_SF);

  Serial.print("LoRa: set tx power to: ");
  Serial.print(RFM95_TX_POWER);
  Serial.println(" dB");
  LoRa.setTxPower(RFM95_TX_POWER, PA_OUTPUT_PA_BOOST_PIN);

  Serial.println("LoRa: enable CRC");
  LoRa.enableCrc();

  LoRa.onReceive(onReceive);
  LoRa_rxMode();

#if defined(LORA_SENDER)
  g_b_enable_send = true; // enable sending initially
#endif
}

void LoRa_rxMode( void ) {
  // LoRa.enableInvertIQ();                // active invert I and Q signals
  LoRa.receive();                       // set receive mode
}

void LoRa_txMode( void ) {
  LoRa.idle();                          // set standby mode
  // LoRa.disableInvertIQ();               // normal mode
}

void LoRa_sendMessage( String message ) {
  LoRa_txMode();                        // set tx mode
  LoRa.beginPacket();                   // start packet
  LoRa.write(g_by_destinationAddress);  // add destination address
  LoRa.write(g_by_localAddress);        // add sender address
  // LoRa.write(g_i_msgCount);             // add message ID

  // function LoRaClass::write(uint8_t byte) can only write 8bit uint
  // solution: write high byte first and then low byte of 16bit uint
  LoRa.write((uint8_t)(g_i_msgCount >> 8 ));  // add message ID (high byte)
  LoRa.write((uint8_t)g_i_msgCount );         // add message ID (low byte)

  LoRa.write(message.length());         // add payload length
  LoRa.print(message);                  // add payload
  LoRa.endPacket();                     // finish packet and send it
  LoRa_rxMode();                        // set rx mode
  g_i_msgCount++;                       // increment message ID
}

void function_lora_send_handler( void ){
  // sender: repeat every 2000 milliseconds
  // receiver: repeat every 200 milliseconds
  // if (runEvery(RFM_TX_INTERVAL) && g_b_enable_send) {
#if defined(LORA_SENDER)
  if (runEvery(RFM_TX_INTERVAL)) {
    // activate sending after timeout is reached for a next try;
    // maybe someone s hearing me this time?
    if (!g_b_enable_send) {
      g_i_rfm_tx_timeout_cnt_curr++;

      if (g_i_rfm_tx_timeout_cnt_curr >= RFM_TX_TIMEOUT_CNT) {
        Serial.println("LoRa: activate sending after timeout is reached for a next try ### ");
        g_b_enable_send = true;
      }
    }
    else {
      // reset timeout counter
      g_i_rfm_tx_timeout_cnt_curr = 0;
      String message = "HeLoRa World! I'm a Node! ";
#elif defined(LORA_RECEIVER)
  if (runEvery(RFM_TX_INTERVAL)) {
    if (g_b_enable_send) {
      String message = "LoRa ACK: message received :) ";
#endif
      message += millis();
      message += " ms";
      // send message
      LoRa_sendMessage(message);

      Serial.print("LoRa: sent message '");
      Serial.print(message);
      Serial.println("'");

#if defined(LORA_SENDER)
      digitalWrite(LED_PIN, LOW); // negative logic: LED on
#elif defined(LORA_RECEIVER)
      digitalWrite(LED_PIN, HIGH); // negative logic: LED off
#endif

      g_b_enable_send = false;    // disable sending

      // set rx mode
      LoRa_rxMode();
    }
// #if defined(LORA_SENDER)
  }
// #endif
}

// Note:  while sending, LoRa radio is not listening for incoming messages.
// Note2: when using the callback method, you can't use any of the Stream
//        functions that rely on the timeout, such as readString, parseInt(), etc.
// Note3: don't send messages in onReceive function (esp8266 crashes and reboots randomly!)
//        => sending of acknowledge messages has to be done outside!
void onReceive(int packetSize) {
  // if there's no packet, return
  if (packetSize == 0) return;

#if defined(LORA_SENDER)
  digitalWrite(LED_PIN, HIGH); // negative logic: LED off
#elif defined(LORA_RECEIVER)
  digitalWrite(LED_PIN, LOW); // negative logic: LED on
#endif

  // read packet header bytes:
  g_by_incomingRecipient = LoRa.read(); // recipient address
  g_by_incomingSender = LoRa.read();    // sender address
  // g_i_incomingMsgId = (int)LoRa.read(); // incoming msg ID

  // function LoRaClass::write(uint8_t byte) can only write 8bit uint
  // solution: read high byte first and then low byte of 16bit uint
  g_i_incomingMsgId = LoRa.read();
  g_i_incomingMsgId = ( g_i_incomingMsgId << 8 ) + LoRa.read();

  g_by_incomingLength = LoRa.read();    // incoming msg length

  g_str_incoming_msg = "";              // clear old message string

  while (LoRa.available()) {                  // can't use readString() in callback, so
    g_str_incoming_msg += (char)LoRa.read();  // add bytes one by one
  }

  // check message length for error
  if (g_by_incomingLength != g_str_incoming_msg.length()) {
    Serial.println("LoRa: error: message length does not match length");
    return; // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (g_by_incomingRecipient != g_by_localAddress && g_by_incomingRecipient != g_by_broadcastAddress) {
    Serial.println("LoRa: This message is not for me.");
    return; // skip rest of function
  }

  g_i_rssi = LoRa.packetRssi();
  g_f_snr = LoRa.packetSnr();

  // // if message is for this device, or broadcast, print details:
  // Serial.println("### LoRa start receiving ###");
  // Serial.println("Received from: 0x" + String(l_by_sender, HEX));
  // Serial.println("Sent to: 0x" + String(l_by_recipient, HEX));
  // Serial.println("Message ID: " + String(g_i_incomingMsgId));
  // Serial.println("Message ID (internal): " + String(g_i_msgCount));
  // Serial.println("Message length: " + String(l_by_incomingLength));
  // Serial.println("Message: " + message);
  // Serial.println("RSSI: " + String(g_i_rssi));
  // Serial.println("Snr: " + String(g_f_snr));
  // Serial.println("### LoRa end receiving ###");

#if defined(LORA_SENDER)
  g_b_enable_send = true;  // enable sending again
#elif defined(LORA_RECEIVER)
  g_b_enable_send = true;  // enable sending (only once for acknowledge)
#endif
}

bool runEvery(unsigned long interval) {
  unsigned long currentMillis = millis();
  if (currentMillis - g_ul_previousMillis_lora >= interval) {
    // save the last time that cycle has been run
    g_ul_previousMillis_lora = currentMillis;

    // Serial.println("### LoRa DEBUG: was here .. ###");

    return true;
  }
  return false;
}

int function_lora_get_rssi( void ){
  return g_i_rssi;
}

float function_lora_get_snr( void ){
  return g_f_snr;
}

int function_lora_get_msgID( void ){
  return g_i_incomingMsgId;
}

int function_lora_get_msgCount( void ){
  return g_i_msgCount;
}

void function_lora_serial_out( void ){
  // serial output lora details
  Serial.println("### LoRa start receiving ###");
  Serial.println("Received from: 0x" + String(g_by_incomingSender, HEX));
  Serial.println("Sent to: 0x" + String(g_by_incomingRecipient, HEX));
  Serial.println("Message ID: " + String(g_i_incomingMsgId));
  Serial.println("Message ID (internal): " + String(g_i_msgCount));
  Serial.println("Message length: " + String(g_by_incomingLength));
  Serial.println("Message: " + g_str_incoming_msg);
  Serial.println("RSSI: " + String(g_i_rssi));
  Serial.println("Snr: " + String(g_f_snr));
  Serial.println("### LoRa end receiving ###");
}




















//
