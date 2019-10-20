#ifndef function_mqtt_hpp
#define function_mqtt_hpp

#include <AsyncMqttClient.h>
#include <Ticker.h>

#include "config.hpp"

void configureMqtt();
void connectToMqtt();
void connectMqttPubTasks();

void onMqttConnect(bool sessionPresent);

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
void deactivateMqtt_reconnectTimer();

void onMqttSubscribe(uint16_t packetId, uint8_t qos);

void onMqttUnsubscribe(uint16_t packetId);

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);

bool stringStartsWith(const char *pre, const char *str);

void onMqttPublish(uint16_t packetId);

void mqtt_set_wifi_rssi_dBm(int rssi);
void mqttPub_wifi_rssi();






#endif // function_mqtt_hpp
