#ifdef LORA_TOPO_TTN  // build only when it is really needed

#include "function_lorawan_ttn.hpp"

//
// Based on the example that uses activation-by-personalization (ABP, preconfiguring a device address and encryption keys)
// https://github.com/matthijskooijman/arduino-lmic/tree/master/examples/ttn-abp
//

#include <hal/hal.h>
#include <SPI.h>      // DHT sensor library by Adafruit

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library

#include <DHT.h>

// Connect pin 1 (on the left) of the sensor to +3.3 - +5.0 V
// Connect pin 2 (data pin) of the sensor to whatever your DHTPIN is (GPIO of esp8266)
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor
// Connect pin 4 (on the right) of the sensor to GROUND
// Let pin 3 unconnected

// See guide for details on sensor wiring and usage:
// https://learn.adafruit.com/dht/overview
DHT dht(DHTPIN, DHTTYPE);

// LoRaWAN NwkSKey, network session key
// This is the default Semtech key, which is used by the early prototype TTN network.
static const PROGMEM u1_t NWKSKEY[16] = { 0x7F, 0xD4, 0x6E, 0xF0, 0xA1, 0xB9, 0xDF, 0x72, 0xBA, 0xE1, 0x51, 0x14, 0xA6, 0x67, 0x5F, 0xFA };

// LoRaWAN AppSKey, application session key
// This is the default Semtech key, which is used by the early prototype TTN network.
static const u1_t PROGMEM APPSKEY[16] = { 0xE8, 0x14, 0xF4, 0xBC, 0xC9, 0x54, 0x85, 0xB3, 0xB8, 0x8E, 0xDE, 0xA6, 0x76, 0x20, 0x44, 0x65 };

// LoRaWAN end-device address (DevAddr)
static const u4_t DEVADDR = 0x26011A99 ; // <-- Change this address for every node!

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

//static uint8_t mydata[] = "ABP node: Hello, world!";
static osjob_t sendjob;

// Schedule TX every this many seconds (might become longer due to duty cycle limitations).
const unsigned TX_INTERVAL = 10; // 10 s

// Pin mapping of LoRa module
const lmic_pinmap lmic_pins = {
    .nss = RFM95_CS,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = RFM95_RST,
    .dio = {RFM95_INT0, RFM95_INT1, LMIC_UNUSED_PIN},      // DO0, DO1 (DO2 is not used)
};

void onEvent (ev_t ev) {
  Serial.print(os_getTime());
  Serial.print(": ");
  switch(ev) {
    case EV_SCAN_TIMEOUT:
      Serial.println(F("EV_SCAN_TIMEOUT"));
      break;
    case EV_BEACON_FOUND:
      Serial.println(F("EV_BEACON_FOUND"));
      break;
    case EV_BEACON_MISSED:
      Serial.println(F("EV_BEACON_MISSED"));
      break;
    case EV_BEACON_TRACKED:
      Serial.println(F("EV_BEACON_TRACKED"));
      break;
    case EV_JOINING:
      Serial.println(F("EV_JOINING"));
      break;
    case EV_JOINED:
      Serial.println(F("EV_JOINED"));
      break;
    case EV_RFU1:
      Serial.println(F("EV_RFU1"));
      break;
    case EV_JOIN_FAILED:
      Serial.println(F("EV_JOIN_FAILED"));
      break;
    case EV_REJOIN_FAILED:
      Serial.println(F("EV_REJOIN_FAILED"));
      break;
    case EV_TXCOMPLETE:
      Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
      digitalWrite(LED_PIN, LOW);   // turn the LED off

      if (LMIC.txrxFlags & TXRX_ACK)
        Serial.println(F("Received ack"));

      if (LMIC.dataLen) {
        Serial.println(F("Received "));
        Serial.println(LMIC.dataLen);
        Serial.println(F(" bytes of payload"));
      }
      // Schedule next transmission
      os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
      break;
    case EV_LOST_TSYNC:
      Serial.println(F("EV_LOST_TSYNC"));
      break;
    case EV_RESET:
      Serial.println(F("EV_RESET"));
      break;
    case EV_RXCOMPLETE:
      // data received in ping slot
      Serial.println(F("EV_RXCOMPLETE"));
      break;
    case EV_LINK_DEAD:
      Serial.println(F("EV_LINK_DEAD"));
      break;
    case EV_LINK_ALIVE:
      Serial.println(F("EV_LINK_ALIVE"));
      break;
    default:
      Serial.println(F("Unknown event"));
      break;
  }
}

void do_send(osjob_t* j) {
  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
  }
  else {
    char char_buffer[8];
    uint8_t ui_buffer[8];
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
      Serial.println(F("Failed to read from DHT sensor!"));
    }
    else {
      dtostrf(t, 2, 1, char_buffer);
      dtostrf(h, 2, 1, &char_buffer[4]);  // write humidity in char array beginning at position 4
      Serial.println(char_buffer);

      memcpy(ui_buffer, char_buffer, 8);  // copy char array into uint8_t array
      Serial.println((char*)ui_buffer);

      Serial.println("");
      Serial.print("Sending - temperature: ");
      Serial.print(t);
      Serial.print(" °C, humidity: ");
      Serial.print(h);
      Serial.println("% rH");

      digitalWrite(LED_PIN, HIGH);   // turn the LED on
    }

    // Prepare upstream data transmission at the next possible time.
    //LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
    LMIC_setTxData2(1, ui_buffer, sizeof(ui_buffer), 0);
    //LMIC_setTxData2(1, (uint8_t*)char_buffer, sizeof((uint8_t*)char_buffer)-1, 0);

    Serial.println(F("Packet queued"));
  }
  // Next TX is scheduled after TX_COMPLETE event.
}

void function_lorawan_ttn_setup( void ) {
  // initialize LED pin as an output.
  pinMode(LED_PIN, OUTPUT);

  // Initialize DHT11 sensor device
  dht.begin();
  //sensor_t dht11_sensor;
  //dht.temperature().getSensor(&dht11_sensor);
  // Set delay between sensor readings based on sensor details.
  //delayMS = dht11_sensor.min_delay / 1000 * 2; // 2000 ms
  //delayMS = 2000;
  //Serial.println(F("------------------------------------"));
  //Serial.print  (F("Read delay:  ")); Serial.print(delayMS); Serial.println(F(" ms"));
  //Serial.println(F("------------------------------------"));

  // LMIC init
  os_init();
  // Reset the MAC state. Session and pending data transfers will be discarded.
  LMIC_reset();

  // Set static session parameters. Instead of dynamically establishing a session
  // by joining the network, precomputed session parameters are be provided.
  #ifdef PROGMEM
  // On AVR, these values are stored in flash and only copied to RAM
  // once. Copy them to a temporary buffer here, LMIC_setSession will
  // copy them into a buffer of its own again.
  uint8_t appskey[sizeof(APPSKEY)];
  uint8_t nwkskey[sizeof(NWKSKEY)];
  memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
  memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
  LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
  #else
  // If not running an AVR with PROGMEM, just use the arrays directly
  LMIC_setSession (0x1, DEVADDR, NWKSKEY, APPSKEY);
  #endif

  #if defined(CFG_eu868)
  // Set up the channels used by the Things Network, which corresponds
  // to the defaults of most gateways. Without this, only three base
  // channels from the LoRaWAN specification are used, which certainly
  // works, so it is good for debugging, but can overload those
  // frequencies, so be sure to configure the full frequency range of
  // your network here (unless your network autoconfigures them).
  // Setting up channels should happen after LMIC_setSession, as that
  // configures the minimal channel set.
  // NA-US channels 0-71 are configured automatically
  LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  // LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
  // LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  // LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  // LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  // LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  // LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  // LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  // LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
  // TTN defines an additional channel at 869.525Mhz using SF9 for class B
  // devices' ping slots. LMIC does not have an easy way to define set this
  // frequency and support for class B is spotty and untested, so this
  // frequency is not configured here.
  #elif defined(CFG_us915)
  // NA-US channels 0-71 are configured automatically
  // but only one group of 8 should (a subband) should be active
  // TTN recommends the second sub band, 1 in a zero based count.
  // https://github.com/TheThingsNetwork/gateway-conf/blob/master/US-global_conf.json
  LMIC_selectSubBand(1);
  #endif

  // Disable link check validation
  LMIC_setLinkCheckMode(0);

  // TTN uses SF9 for its RX2 window.
  LMIC.dn2Dr = DR_SF9;

  // Set data rate and transmit power for uplink (note: txpow seems to be ignored by the library)
  LMIC_setDrTxpow(DR_SF7,14);

  // Start job
  do_send(&sendjob);
}

void function_lorawan_ttn_send_handler( void ) {
  os_runloop_once();  // call LMIC task
}















#endif // LORA_TOPO_TTN


//
