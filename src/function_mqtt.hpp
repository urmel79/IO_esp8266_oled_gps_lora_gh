#ifndef function_mqtt_hpp
#define function_mqtt_hpp

#include <AsyncMqttClient.h>
#include <Ticker.h>

#include "config.hpp"

void connectToMqtt();

void onMqttConnect(bool sessionPresent);

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);

void onMqttSubscribe(uint16_t packetId, uint8_t qos);

void onMqttUnsubscribe(uint16_t packetId);

void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);

bool stringStartsWith(const char *pre, const char *str);

void onMqttPublish(uint16_t packetId);








#endif // function_mqtt_hpp
