#include "function_mqtt.hpp"

#include <ArduinoJson.h>

#include "function_wifi.hpp"
#include "function_gps.hpp"

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

Ticker mqtt_pub_wifi_quality;
Ticker mqtt_pub_gps_json;

// Schedule mqtt publishing every this many seconds
const unsigned MQTT_PUB_INTERVAL = 4; // 2 s

// Ticker mqtt_pub_sensor_measurements;

int g_i_rssi_dBm;

bool g_b_gps_valid;
String g_str_sensor;
String g_str_time;
String g_str_location;
String g_str_icon;
String g_str_iconColor;
int g_i_satellites;
double g_d_altitude;
double g_d_latitude;
double g_d_longitude;
float g_f_lat_avg;
float g_f_lng_avg;
float g_f_alt_avg;
float g_f_lat_median;
float g_f_lng_median;
float g_f_alt_median;

void function_mqtt_setup() {
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
}

void function_mqtt_connect() {
  function_gps_disable_Rx();

  Serial.print("[MQTT] Connecting to MQTT host: ");
  Serial.println(String(MQTT_HOST));
  mqttClient.connect();
}

void function_mqtt_connect_PubTasks() {
  mqtt_pub_wifi_quality.attach(MQTT_PUB_INTERVAL, mqttPub_wifi_rssi);
  mqtt_pub_gps_json.attach(MQTT_PUB_INTERVAL, mqttPub_gps_json);
}

void function_mqtt_disconnect_PubTasks() {
  mqtt_pub_wifi_quality.detach();
  mqtt_pub_gps_json.detach();
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("[MQTT] Connected to MQTT.");
  Serial.print("[MQTT] Session present: ");
  Serial.println(sessionPresent);

  // Enable interrupts on the rx pin after mqtt (re)connect attempt
  function_gps_enable_Rx();
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  function_gps_disable_Rx();

  Serial.println("[MQTT] Disconnected from MQTT.");

  if (get_wifi_isConnected()) {
    mqttReconnectTimer.once(2, function_mqtt_connect);
  }
}

void deactivateMqtt_reconnectTimer() {
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi

  // also publishing data over MQTT does not make any sense at the moment ...
  mqtt_pub_wifi_quality.detach();
  mqtt_pub_gps_json.detach();
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("[MQTT] Subscribe acknowledged.");
  Serial.print("[MQTT] packetId: ");
  Serial.println(packetId);
  Serial.print("[MQTT] qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("[MQTT] Unsubscribe acknowledged.");
  Serial.print("[MQTT] packetId: ");
  Serial.println(packetId);
}

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  payload: ");
  Serial.println(payload);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
}

// payload is really dirty at the end, so string has to start with search pattern
bool stringStartsWith(const char *pre, const char *str) {
  size_t lenpre = strlen(pre),
         lenstr = strlen(str);
  return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("[MQTT] Publish acknowledged.");
  Serial.print("[MQTT] packetId: ");
  Serial.println(packetId);
}

void mqtt_set_wifi_rssi_dBm(int rssi) {
  g_i_rssi_dBm = rssi;
}

void mqttPub_wifi_rssi() {
  // if mqtt not connected, return asap to prevent interrupt conflicts
  if (!mqttClient.connected()) {
    return;
  }

  // Rx interrupts are conflicting with mqtt ticker event => disable serial Rx!
  function_gps_disable_Rx();

  bool send_success;

  // convert int to string
  String str_wifi_rssi_dBm = String(g_i_rssi_dBm);

  String topic_pub = String(MQTT_ROOT_TOPIC);
  topic_pub += "/wifi/rssi";

  send_success = mqttClient.publish(topic_pub.c_str(), 0, true, str_wifi_rssi_dBm.c_str());

  if (send_success) {
    Serial.print("[MQTT] Wi-Fi signal level = ");
    Serial.println(str_wifi_rssi_dBm);
  }
  else Serial.println("[MQTT] Error sending message.");

  // Enable interrupts on the rx pin after mqtt publishing
  function_gps_enable_Rx();
}

void mqtt_set_gps_valid(bool gps_valid) {
  g_b_gps_valid = gps_valid;
}

void mqtt_set_gps_json( String sensor, String time, String location, String icon,
                        String iconColor, int satellites, double altitude,
                        int wifi_rssi, double latitude, double longitude,
                        float lat_avg, float lng_avg, float alt_avg,
                        float lat_median, float lng_median, float alt_median ) {
  g_str_sensor = sensor;
  g_str_time = time;
  g_str_location = location;
  g_str_icon = icon;
  g_str_iconColor = iconColor;
  g_i_satellites = satellites;
  g_d_altitude = altitude;
  g_i_rssi_dBm = wifi_rssi;
  g_d_latitude = latitude;
  g_d_longitude = longitude;
  g_f_lat_avg = lat_avg;
  g_f_lng_avg = lng_avg;
  g_f_alt_avg = alt_avg;
  g_f_lat_median = lat_median;
  g_f_lng_median = lng_median;
  g_f_alt_median = alt_median;
}

void mqttPub_gps_json() {
  // if mqtt not connected, return asap to prevent interrupt conflicts
  if (!mqttClient.connected()) {
    return;
  }

  if (g_b_gps_valid) {

    bool send_success;
    String l_res;

    // Rx interrupts are conflicting with mqtt ticker event => disable serial Rx!
    function_gps_disable_Rx();

    // Allocate the JSON document
    //
    // Inside the brackets, 200 is the RAM allocated to this document.
    // Don't forget to change this value to match your requirement.
    // Use https://arduinojson.org/v6/assistant to compute the capacity.
    // StaticJsonDocument<200> doc;

    // StaticJsonObject allocates memory on the stack, it can be
    // replaced by DynamicJsonDocument which allocates in the heap.
    //
    // Wichtig: muss bei Erweiterung des Json-Dokuments immer angepasst werden!
    //          Es müssen 3 weitere Objektplätze reserviert werden, damit das
    //          JsonDocument vollständig übertragen wird. Warum??
    // Use https://arduinojson.org/v6/assistant to compute the capacity.
    const size_t capacity = 3*JSON_ARRAY_SIZE(3+3) + JSON_OBJECT_SIZE(10+3);
    DynamicJsonDocument JsonDoc(capacity);

    JsonObject Json_rootObj = JsonDoc.to<JsonObject>();

    // Add values in the document
    Json_rootObj["sensor"] = g_str_sensor;
    Json_rootObj["time"] = g_str_time;
    Json_rootObj["location"] = g_str_location;
    Json_rootObj["icon"] = g_str_icon;
    Json_rootObj["iconColor"] = g_str_iconColor;
    Json_rootObj["satellites"] = g_i_satellites;
    // Json_rootObj["altitude"] = g_d_altitude;
    Json_rootObj["wifi_rssi"] = g_i_rssi_dBm;

    // Add an location array (raw values).
    JsonArray gps_loc = Json_rootObj.createNestedArray("gps_loc");

    l_res = convertDouble2String(g_d_latitude, 12, 9); // convert double to string with desired precision 9
    gps_loc.add(l_res);
    l_res = convertDouble2String(g_d_longitude, 12, 9); // convert double to string with desired precision 9
    gps_loc.add(l_res);
    l_res = convertDouble2String(g_d_altitude, 12, 1); // convert double to string with desired precision 1
    gps_loc.add(l_res);

    // Add an location array (avg).
    JsonArray gps_loc_avg = Json_rootObj.createNestedArray("gps_loc_avg");

    l_res = convertDouble2String(g_f_lat_avg, 12, 9); // convert double to string with desired precision 9
    gps_loc_avg.add(l_res);
    l_res = convertDouble2String(g_f_lng_avg, 12, 9); // convert double to string with desired precision 9
    gps_loc_avg.add(l_res);
    l_res = convertDouble2String(g_f_alt_avg, 12, 1); // convert double to string with desired precision 1
    gps_loc_avg.add(l_res);

    // Add an location array (median).
    JsonArray gps_loc_median = Json_rootObj.createNestedArray("gps_loc_median");

    l_res = convertDouble2String(g_f_lat_median, 12, 9); // convert double to string with desired precision 9
    gps_loc_median.add(l_res);
    l_res = convertDouble2String(g_f_lng_median, 12, 9); // convert double to string with desired precision 9
    gps_loc_median.add(l_res);
    l_res = convertDouble2String(g_f_alt_median, 12, 1); // convert double to string with desired precision 1
    gps_loc_median.add(l_res);

    // // Generate the minified JSON and send it to the Serial port.
    // serializeJsonPretty(Json_rootObj, Serial);
    // Serial.println("");

    String topic_pub = String(MQTT_ROOT_TOPIC);
    topic_pub += "/lora/gps";
    // String mqttJsonPayload;
    char mqttJsonPayload[512];
    // save a few CPU cycles by passing the size of the payload to publish()
    size_t n = serializeJson(Json_rootObj, mqttJsonPayload);

    send_success = mqttClient.publish(topic_pub.c_str(), 0, true, mqttJsonPayload, n);

    if (send_success) {
      Serial.print("[MQTT] JSON payload = ");
      Serial.println(mqttJsonPayload);
    }
    else Serial.println("[MQTT] Error sending message.");

    // Enable interrupts on the rx pin after mqtt publishing
    function_gps_enable_Rx();
  }
}

String convertDouble2String(double value, uint flt_positions, uint precision) {
  char str_value[21];     //result string 20 positions + '\0' at the end

  if (flt_positions < precision) precision = flt_positions + 2; // sanitize precision
  if (flt_positions > 20) flt_positions = 20; // validate flt_positions (max 20 digits)
  if (precision > 10) precision = 10; // validate precision (max 10 precision digits)
  dtostrf( value, flt_positions, precision, str_value ); // convert float/double to string with desired precision

  return str_value;
}














//
