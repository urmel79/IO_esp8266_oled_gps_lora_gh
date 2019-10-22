#include "function_mqtt.hpp"

#include <ArduinoJson.h>

#include "function_wifi.hpp"

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

Ticker mqtt_pub_wifi_quality;
Ticker mqtt_pub_gps_json;

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

void configureMqtt() {
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
}

void connectToMqtt() {
  Serial.print("Connecting to MQTT host: ");
  Serial.println(String(MQTT_HOST));
  mqttClient.connect();
}

void connectMqttPubTasks() {
  mqtt_pub_wifi_quality.attach(2, mqttPub_wifi_rssi);
  mqtt_pub_gps_json.attach(2, mqttPub_gps_json);
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);

//   String topic_sub = String(MQTT_ROOT_TOPIC);
//   topic_sub += "/lighting/command";
// //   Serial.println(String(MQTT_ROOT_TOPIC));
//   // packetIdSub = mqttClient.subscribe(topic_sub.c_str(), 2);
//   mqttClient.subscribe(topic_sub.c_str(), 2);
//
//   Serial.print("Subscribing at QoS 0, topic: ");
//   Serial.println(String(topic_sub));
//
//   // get current sealevel pressure (NN)
//   // packetIdSub = mqttClient.subscribe("openweather/press_nn", 0);
//   mqttClient.subscribe("openweather/press_nn", 0);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");

  if (get_wifi_isConnected()) {
    mqttReconnectTimer.once(2, connectToMqtt);
  }
}

void deactivateMqtt_reconnectTimer() {
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
}

void onMqttSubscribe(uint16_t packetId, uint8_t qos) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  Serial.print("  qos: ");
  Serial.println(qos);
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
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

//   // If a message is received on the topic room/lamp, you check if the message is either on or off. Turns the lamp GPIO according to the message
//   String topic_sub = String(MQTT_ROOT_TOPIC);
//   topic_sub += "/lighting/command";
//
//   //if(!strcmp(topic, topic_sub.c_str())) {
//   if(String(topic) == topic_sub) {
//     Serial.print("Changing Room lamp to ");
//     if(stringStartsWith("on", payload)) {
// #if defined(DEAUTHER_OLED)
//   #if !defined(BOX_HAS_PIR)
//       digitalWrite(g_ci_lamp_gpio, LOW); // negative logic on internal Wifi LED
//   #endif
// #elif defined(WEMOS_D1_MINI_PRO)
//   #if !defined(BOX_HAS_PIR)
//       digitalWrite(g_ci_lamp_gpio, LOW); // negative logic on internal Wifi LED
//   #endif
//
//       // RCSwitch on:
//       // The first parameter represents the setting of the first 5 DIP switches.
//       // In this example it's ON-ON-OFF-OFF-ON.
//       //
//       // The second parameter represents the setting of the last 5 DIP switches (channel).
//       // In this example the last 5 DIP switches are OFF-ON-OFF-ON-OFF.
//       // channels A: 10000, B: 01000, C: 00100
//       mySwitch.switchOn("10101", "10000");  // Steckdose A
// #elif defined(NODE_MCU_V2)
//   #if !defined(BOX_HAS_PIR)
//       digitalWrite(g_ci_lamp_gpio, HIGH);
//   #endif
//
//       // RCSwitch on:
//       // The first parameter represents the setting of the first 5 DIP switches.
//       // In this example it's ON-ON-OFF-OFF-ON.
//       //
//       // The second parameter represents the setting of the last 5 DIP switches (channel).
//       // In this example the last 5 DIP switches are OFF-ON-OFF-ON-OFF.
//       // channels A: 10000, B: 01000, C: 00100
//       mySwitch.switchOn("10101", "00100");  // Steckdose C
// #endif
//       Serial.println("LED On");
//       g_b_lamp_status = true;
//
//       String topic_pub_led_status = String(MQTT_ROOT_TOPIC) + "/lighting/status";
//       mqttClient.publish(topic_pub_led_status.c_str(), 2, true, "on"); // publish lamp status to set the switch in gui properly
//     }
//     else if(stringStartsWith("off", payload)) {
// #if defined(DEAUTHER_OLED)
//   #if !defined(BOX_HAS_PIR)
//       digitalWrite(g_ci_lamp_gpio, HIGH); // negative logic on internal Wifi LED
//   #endif
// #elif defined(WEMOS_D1_MINI_PRO)
//   #if !defined(BOX_HAS_PIR)
//       digitalWrite(g_ci_lamp_gpio, HIGH); // negative logic on internal Wifi LED
//   #endif
//
//       // RCSwitch off
//       mySwitch.switchOff("10101", "10000");  // Steckdose A
// #elif defined(NODE_MCU_V2)
//   #if !defined(BOX_HAS_PIR)
//       digitalWrite(g_ci_lamp_gpio, LOW);
//   #endif
//
//       // RCSwitch off
//       mySwitch.switchOff("10101", "00100");  // Steckdose C
// #endif
//       Serial.println("LED Off");
//       g_b_lamp_status = false;
//
//       String topic_pub_led_status = String(MQTT_ROOT_TOPIC) + "/lighting/status";
//       mqttClient.publish(topic_pub_led_status.c_str(), 2, true, "off"); // publish lamp status to set the switch in gui properly
//     }
//   }
//   else if(String(topic) == "openweather/press_nn") {
// //     Serial.print("##### press_nn: ");
// //     Serial.println(payload);
//
//     // @TODO: ziemlicher Misthack!! => wenn Druck nur 3-stellig => wird evtl. wieder durch Zeichensalat verseucht ...
//     String payload_cut = String(payload);
//     payload_cut.remove(4); // Remove from from index=4 through the end of the string
//
// //     Serial.print("##### Cleaned press_nn: ");
// //     Serial.println(payload_cut.toInt());
//
//     g_i_sealevel_pressure_nn = payload_cut.toInt();
//   }
}

// payload is really dirty at the end, so string has to start with search pattern
bool stringStartsWith(const char *pre, const char *str) {
  size_t lenpre = strlen(pre),
         lenstr = strlen(str);
  return lenstr < lenpre ? false : memcmp(pre, str, lenpre) == 0;
}

void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

void mqtt_set_wifi_rssi_dBm(int rssi) {
  g_i_rssi_dBm = rssi;
}

void mqttPub_wifi_rssi() {
  bool send_success;

  // convert int to string
  String str_wifi_rssi_dBm = String(g_i_rssi_dBm);

  String topic_pub = String(MQTT_ROOT_TOPIC);
  topic_pub += "/wifi/rssi";

  if (mqttClient.connected()) {
    send_success = mqttClient.publish(topic_pub.c_str(), 0, true, str_wifi_rssi_dBm.c_str());

    if (send_success) {
      Serial.print("MQTT: Wi-Fi signal level = ");
      Serial.println(str_wifi_rssi_dBm);
    }
    else Serial.println("MQTT: Error sending message.");
  }
}

void mqtt_set_gps_valid(bool gps_valid) {
  g_b_gps_valid = gps_valid;
}

void mqtt_set_gps_json(String sensor, String time, String location, String icon, String iconColor, int satellites, double altitude, int wifi_rssi, double latitude, double longitude) {
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
}

void mqttPub_gps_json() {
  bool send_success;

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
  //          Es müssen 2 weitere Objektplätze reserviert werden, damit das
  //          JsonDocument vollständig übertragen wird. Warum??
  // Use https://arduinojson.org/v6/assistant to compute the capacity.
  const size_t capacity = JSON_ARRAY_SIZE(2+1) + JSON_OBJECT_SIZE(9+3);
  DynamicJsonDocument JsonDoc(capacity);

  JsonObject Json_rootObj = JsonDoc.to<JsonObject>();

  // Add values in the document
  Json_rootObj["sensor"] = g_str_sensor;
  Json_rootObj["time"] = g_str_time;
  Json_rootObj["location"] = g_str_location;
  Json_rootObj["icon"] = g_str_icon;
  Json_rootObj["iconColor"] = g_str_iconColor;
  Json_rootObj["satellites"] = g_i_satellites;
  Json_rootObj["altitude"] = g_d_altitude;
  Json_rootObj["wifi_rssi"] = g_i_rssi_dBm;

  // Add an array.
  JsonArray gps_loc = Json_rootObj.createNestedArray("gps_loc");
  // gps_loc.add(51.003886167);
  // gps_loc.add(13.686812667);
  gps_loc.add(g_d_latitude);
  gps_loc.add(g_d_longitude);

  // // Generate the minified JSON and send it to the Serial port.
  // serializeJsonPretty(Json_rootObj, Serial);
  // Serial.println("");

  String topic_pub = String(MQTT_ROOT_TOPIC);
  topic_pub += "/lora/gps";
  // String mqttJsonPayload;
  char mqttJsonPayload[512];
  // save a few CPU cycles by passing the size of the payload to publish()
  size_t n = serializeJson(Json_rootObj, mqttJsonPayload);

  if (mqttClient.connected() && g_b_gps_valid) {
    send_success = mqttClient.publish(topic_pub.c_str(), 0, true, mqttJsonPayload, n);

    if (send_success) {
      Serial.print("MQTT: JSON payload = ");
      Serial.println(mqttJsonPayload);
    }
    else Serial.println("MQTT: Error sending message.");
  }
}
















//
