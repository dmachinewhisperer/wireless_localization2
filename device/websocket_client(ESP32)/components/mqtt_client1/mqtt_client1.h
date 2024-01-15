#ifndef _MQTT_CLIENT1_H_
#define _MQTT_CLIENT1_H_

#include "mqtt_client.h"

#define PRI_NODE_TOPIC  "devices/node1"
#define SEC_NODE_TOPIC1 "devices/node2"

extern esp_mqtt_client_handle_t client;

void start_mqtt_client(void);

void publish(char *topic, char *payload);

#endif

