#include "function_mqtt.hpp"

AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

Ticker mqtt_pub_wifi_quality;

Ticker mqtt_pub_sensor_measurements;

void connectToMqtt() {
  Serial.print("Connecting to MQTT host: ");
  Serial.println(String(MQTT_HOST));
  mqttClient.connect();
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

  // if (WiFi.isConnected()) {
  //   mqttReconnectTimer.once(2, connectToMqtt);
  // }
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
