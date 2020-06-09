#ifdef LORA_TOPO_TTN  // build only when it is really needed

#include "function_lorawan_ttn.hpp"

#include <Ticker.h>

// #include "function_wifi.hpp"
#include "wifi_creds.hpp"
#include "function_gps.hpp"

//
// Based on the example that uses activation-by-personalization (ABP, preconfiguring a device address and encryption keys)
// https://github.com/matthijskooijman/arduino-lmic/tree/master/examples/ttn-abp

// decode the LoRa payload in TTN with following JavaScript:
// 'Applications | <app id> | Payload Formats' (custom decoder)
/*
function Decoder(bytes, port) {

  var newLoRaMsg = {};

  if (port === 1) {
    newLoRaMsg.climate = {
      "temperature [°C]": String.fromCharCode.apply(null,bytes).substring(0,4),
      "humidity [% rH]": String.fromCharCode.apply(null,bytes).substring(4,8)
    };

    newLoRaMsg.gps_avg = {
      "latitude [°]": String.fromCharCode.apply(null,bytes).substring(8,17),
      "longitude [°]": String.fromCharCode.apply(null,bytes).substring(17,26)
    };
  }

  return newLoRaMsg;
}
*/

#include <hal/hal.h>
#include <SPI.h>

// REQUIRES the following Arduino libraries:
// - DHT Sensor Library: https://github.com/adafruit/DHT-sensor-library
// DHT sensor library by Adafruit
#include <DHT.h>

// Connect pin 1 (on the left) of the sensor to +3.3 - +5.0 V
// Connect pin 2 (data pin) of the sensor to whatever your DHTPIN is (GPIO of esp8266)
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor
// Connect pin 4 (on the right) of the sensor to GROUND
// Let pin 3 unconnected

// See guide for details on sensor wiring and usage:
// https://learn.adafruit.com/dht/overview
DHT dht(DHTPIN, DHTTYPE);

// Important:
// LoRaWAN 'Device Address', 'Network Session Key' and 'App Session Key' are defined in 'wifi_creds.hpp' for security reasons

// LoRaWAN NwkSKey, network session key
// This is the default Semtech key, which is used by the early prototype TTN network.
// static const PROGMEM u1_t LORA_TTN_NWKSKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// LoRaWAN AppSKey, application session key
// This is the default Semtech key, which is used by the early prototype TTN network.
// static const PROGMEM u1_t LORA_TTN_APPSKEY[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

// LoRaWAN end-device address (DevAddr)
// Change this address for every node!
// static const u4_t LORA_TTN_DEVADDR = 0x00000000;

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in config.h, otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

//static uint8_t mydata[] = "ABP node: Hello, world!";
static osjob_t sendjob;

Ticker LoRa_Tx_watchdog;

// Schedule TX every this many seconds (might become longer due to duty cycle limitations).
const unsigned TX_INTERVAL = 10; // 10 s
bool g_b_LoRaEvent_enabled = false;

// Pin mapping of LoRa module
const lmic_pinmap lmic_pins = {
    .nss = RFM95_CS,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = RFM95_RST,
    .dio = {RFM95_INT0, RFM95_INT1, LMIC_UNUSED_PIN},      // DO0, DO1 (DO2 is not used)
};

float g_f_latitude_avg;
float g_f_longitude_avg;

void onEvent(ev_t ev) {
  // if deactivated, return asap to prevent interrupt conflicts
  if (!g_b_LoRaEvent_enabled) {
    return;
  }

  // // deactivate interrupts from Wifi event handling
  // function_wifiEvent_disable();

  // Serial.print(os_getTime());
  // Serial.print(": ");
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
      Serial.println("[LORA] EV_TXCOMPLETE (includes waiting for RX windows)");
      digitalWrite(LED_PIN, LOW);   // turn the LED off

      // deactivate LoRa Tx watchdog
      LoRa_Tx_watchdog.detach();

      if (LMIC.txrxFlags & TXRX_ACK)
        Serial.println("[LORA] Received ack");

      if (LMIC.dataLen) {
        Serial.print("[LORA] Received ");
        Serial.print(LMIC.dataLen);
        Serial.println(" bytes of payload");
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

  // // activate interrupts from Wifi again ...
  // function_wifiEvent_enable();
}

void do_send(osjob_t* j) {
  // if deactivated, return asap to prevent interrupt conflicts
  if (!g_b_LoRaEvent_enabled) {
    return;
  }

  // Check if there is not a current TX/RX job running
  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println(F("OP_TXRXPEND, not sending"));
  }
  else {
    float t = dht.readTemperature();
    float h = dht.readHumidity();

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) ||
        isnan(g_f_latitude_avg) || isnan(g_f_longitude_avg)) { // average values will always be available ...
      Serial.println(F("Failed to read from DHT sensor or valid GPS sensor data are not available!"));
    }
    else {
      // Serial.println("### LORA Start ### ");
      Serial.print("[LORA] Sending - temperature: ");
      Serial.print(t);
      Serial.print(" *C, humidity: ");
      Serial.print(h);
      Serial.println("% rH");

      char char_buffer[26]; // 8 digits for DHT11 and 18 digits for GPS sensor
      uint8_t ui_buffer[26];

      // write temperature in char array beginning at position 0
      dtostrf(t, 4, 1, char_buffer);
      // write humidity in char array beginning at position 4
      dtostrf(h, 4, 1, &char_buffer[4]);
      // write latitude in char array beginning at position 8
      dtostrf(g_f_latitude_avg, 9, 6, &char_buffer[8]);
      // write longitude in char array beginning at position 17
      dtostrf(g_f_longitude_avg, 9, 6, &char_buffer[17]);
      Serial.print("[LORA] ");
      Serial.println(char_buffer);

      memcpy(ui_buffer, char_buffer, strlen(char_buffer));  // copy char array into uint8_t array

      // Serial.println((char*)ui_buffer);
      // Serial.println("### LORA End ###");

      // Prepare upstream data transmission at the next possible time.
      LMIC_setTxData2(1, ui_buffer, sizeof(ui_buffer), 0); // sending data to FPort 1
    }

    digitalWrite(LED_PIN, HIGH);   // turn the LED on

    Serial.println(F("[LORA] Packet queued"));
  }
  // start LoRa Tx watchdog
  LoRa_Tx_watchdog.once(2*TX_INTERVAL, function_lora_reactivate_Tx);

  // Next TX is scheduled after TX_COMPLETE event.
}

void function_lorawan_ttn_setup( void ) {
  // initialize LED pin as an output.
  pinMode(LED_PIN, OUTPUT);

  // enable LoRaWAN event and transmission handling
  g_b_LoRaEvent_enabled = true;

  // Initialize DHT11 sensor device
  dht.begin();

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
  uint8_t appskey[sizeof(LORA_TTN_APPSKEY)];
  uint8_t nwkskey[sizeof(LORA_TTN_NWKSKEY)];
  memcpy_P(appskey, LORA_TTN_APPSKEY, sizeof(LORA_TTN_APPSKEY));
  memcpy_P(nwkskey, LORA_TTN_NWKSKEY, sizeof(LORA_TTN_NWKSKEY));
  LMIC_setSession (0x1, LORA_TTN_DEVADDR, nwkskey, appskey);
#else
  // If not running an AVR with PROGMEM, just use the arrays directly
  LMIC_setSession (0x1, LORA_TTN_DEVADDR, LORA_TTN_NWKSKEY, LORA_TTN_APPSKEY);
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
  LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI);      // g-band
  LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);      // g-band
  LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK,  DR_FSK),  BAND_MILLI);      // g2-band
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
  // LMIC_setDrTxpow(DR_SF7,14);
  LMIC_setDrTxpow(DR_SF7,20);

  // Start job
  do_send(&sendjob);
}

void function_lorawan_ttn_send_handler( void ) {
  os_runloop_once();  // call LMIC task
}

void function_lorawan_ttn_send_gps(float lat_avg, float lng_avg) {
  g_f_latitude_avg = lat_avg;
  g_f_longitude_avg = lng_avg;
}

void function_LoRaEvent_disable( void ) {
  g_b_LoRaEvent_enabled = false;
  // deactivate LoRa Tx watchdog
  LoRa_Tx_watchdog.detach();
}

void function_LoRaEvent_enable( void ) {
  g_b_LoRaEvent_enabled = true;
  // start LoRa Tx watchdog
  LoRa_Tx_watchdog.once(2*TX_INTERVAL, function_lora_reactivate_Tx);
}

void function_lora_reactivate_Tx( void ) {
  digitalWrite(LED_PIN, LOW);   // turn the LED off

  // reactivating next transmission
  os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);

  // Enable interrupts on the rx pin after wifi (re)connect attempt
  function_gps_enable_Rx();
}





#endif // LORA_TOPO_TTN


//
