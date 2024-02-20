#ifndef _MQTT_CLIENT1_H_
#define _MQTT_CLIENT1_H_

#include "mqtt_client.h"


extern esp_mqtt_client_handle_t client;


void start_mqtt_client(char *endpoint); 

void publish(char *topic, char *payload);

#endif

