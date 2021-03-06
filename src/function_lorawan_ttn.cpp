#if defined(LORA_TOPO_TTN)  // build only when it is really needed

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

      var uptime_s = parseInt(String.fromCharCode.apply(null,bytes).substring(26,35), 10); // 10: base is decimal
      //var uptime_s = 184703; // for testing

      var days    = Math.floor(uptime_s / 3600 / 24);
      var hours   = Math.floor((uptime_s - (days * 3600 * 24)) / 3600);
      var minutes = Math.floor((uptime_s - (days * 3600 * 24) - (hours * 3600)) / 60);
      var seconds = uptime_s - (days * 3600 * 24) - (hours * 3600) - (minutes * 60);

      if (days    < 10) {days    = "0"+days;}
      if (hours   < 10) {hours   = "0"+hours;}
      if (minutes < 10) {minutes = "0"+minutes;}
      if (seconds < 10) {seconds = "0"+seconds;}

      var uptime_str = days+'d '+hours+':'+minutes+':'+seconds;

      newLoRaMsg.uptime = {
        "uptime": uptime_str,
        "uptime [s]": uptime_s
      };
    }

    return newLoRaMsg;
}
*/

#include <hal/hal.h>
#include <SPI.h>

#if defined(BOX_HAS_DTH11)
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

#elif defined(BOX_HAS_SHT35)
  #include "function_sht3x.hpp"

  bool g_b_iicSHT35_connected = false;
#endif

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
    Serial.println("[LORA] OP_TXRXPEND, not sending");
  }
  else {

#if defined(BOX_HAS_DTH11)
    // read temperature and humidity from DHT11 sensor
    float t = dht.readTemperature();
    float h = dht.readHumidity();

#elif defined(BOX_HAS_SHT35)
    // read temperature and humidity from SHT35 sensor
    float t = function_SHTx_get_temperature();
    float h = function_SHTx_get_humidity();
#endif

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t)) {
#if defined(BOX_HAS_DTH11)
      Serial.println("[LORA] Failed to read from DHT11 sensor!");
#elif defined(BOX_HAS_SHT35)
      Serial.println("[LORA] Failed to read from SHT35 sensor!");
#endif
    }
    else {
      Serial.print("[LORA] Temperature: ");
      Serial.print(t);
      Serial.print(" *C, humidity: ");
      Serial.print(h);
      Serial.println("% rH");

      // get uptime [ms]
      unsigned long l_ul_esp_uptime_s = millis() / 1000;
      String l_str_esp_uptime = function_convert_uptime2string(l_ul_esp_uptime_s);

      Serial.print("[LORA] Uptime: ");
      Serial.print(l_str_esp_uptime);
      Serial.print(" or ");
      Serial.print(l_ul_esp_uptime_s);
      Serial.println(" s");

      // 9 digits for SHT35 (or DHT11), 18 digits for GPS sensor and 9 digits for uptime seconds
      char char_buffer[36];
      uint8_t ui_buffer[36];

      // write temperature in char array beginning at position 0
      // layout: 2 digits, comma, 2 decimals = 5 chars
      dtostrf(t, 5, 2, char_buffer);
      // write humidity in char array beginning at position 5
      // layout: 2 digits, comma, 1 decimal = 4 chars
      dtostrf(h, 4, 1, &char_buffer[5]);
      // write latitude in char array beginning at position 9
      dtostrf(g_f_latitude_avg, 9, 6, &char_buffer[9]);
      // write longitude in char array beginning at position 18
      dtostrf(g_f_longitude_avg, 9, 6, &char_buffer[18]);
      // write uptime seconds in char array beginning at position 27
      dtostrf(l_ul_esp_uptime_s, 9, 0, &char_buffer[27]);

      Serial.print("[LORA] ");
      Serial.println(char_buffer);

      memcpy(ui_buffer, char_buffer, strlen(char_buffer));  // copy char array into uint8_t array

      // Prepare upstream data transmission at the next possible time.
      LMIC_setTxData2(1, ui_buffer, sizeof(ui_buffer), 0); // sending data to FPort 1

      digitalWrite(LED_PIN, HIGH);   // turn the LED on

      Serial.println(F("[LORA] Packet queued"));
    }
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

#if defined(BOX_HAS_DTH11)
  // Initialize DHT11 sensor device
  dht.begin();

#elif defined(BOX_HAS_SHT35)
  // Initialize SHT35 sensor device
  g_b_iicSHT35_connected = function_SHTx_connect_iic();
#endif

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

  Serial.println("[LORA] Watchdog was triggered!");

  if (LMIC.opmode & OP_TXRXPEND) {
    Serial.println("[LORA] Flag 'OP_TXRXPEND' was not reset");
    Serial.println("[LORA] Re-initializing LoRa connection and send job");
    function_lorawan_ttn_setup();

    // probably the blocking delay() function is no good idea - anyway ...
    delay(500);
  }
  else {
    Serial.println("[LORA] Reactivating next transmission");
    os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
  }

  // Enable interrupts on the rx pin after wifi (re)connect attempt
  function_gps_enable_Rx();
}

String function_convert_uptime2string(unsigned long time_s) {
  static char l_char_time[32];

  long l_l_days = 0;
  long l_l_hours = 0;
  long l_l_mins = 0;
  long l_l_secs = time_s;
  // l_l_secs = time_ms / 1000; //convect milliseconds to seconds
  l_l_mins = l_l_secs / 60;                 //convert seconds to minutes
  l_l_hours = l_l_mins / 60;                //convert minutes to hours
  l_l_days = l_l_hours / 24;                //convert hours to days
  l_l_secs = l_l_secs - (l_l_mins * 60);    //subtract the coverted seconds to minutes in order to display 59 secs max
  l_l_mins = l_l_mins - (l_l_hours * 60);   //subtract the coverted minutes to hours in order to display 59 minutes max
  l_l_hours = l_l_hours - (l_l_days * 24);  //subtract the coverted hours to days in order to display 23 hours max

  sprintf(l_char_time, "%2.2dd %2.2d:%2.2d:%2.2d", (int)l_l_days, (int)l_l_hours, (int)l_l_mins, (int)l_l_secs);

  return (String)l_char_time;
}




#endif // LORA_TOPO_TTN


//
