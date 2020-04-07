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

void mqtt_set_gps_valid(bool gps_valid);
void mqtt_set_gps_json( String sensor, String time, String location, String icon,
                        String iconColor, int satellites, double altitude,
                        int wifi_rssi, double latitude, double longitude,
                        float lat_avg, float lng_avg, float alt_avg,
                        float lat_median, float lng_median, float alt_median );
void mqttPub_gps_json();

String convertDouble2String(double value, uint flt_positions, uint precision);






#endif // function_mqtt_hpp
