#ifndef _WS_CLIENT_H_
#define _WS_CLIENT_H_

#include <inttypes.h>
#include <stdio.h>

#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "protocol_examples_common.h"

void start_websocket_client(void); 

#endif