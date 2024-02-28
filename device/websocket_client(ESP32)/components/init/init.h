#ifndef _INIT_H_
#define  _INIT_H_
#include <stdint.h>

#include "freertos/semphr.h"
#include "ssd1306.h"

#define CONFIG_SDA_GPIO     21
#define CONFIG_SCL_GPIO     22
#define CONFIG_RESET_GPIO   23

extern char self_id[20];
extern char ext1_id[20];

extern char mqtt_broker_endpoint[128];
extern char ws_server_endpoint[128];

extern SemaphoreHandle_t xSemaphore;
extern SSD1306_t dev;

extern TaskHandle_t task_get_endpoints_xhandle;

void init_display();
uint8_t set_device_public_id();
void task_get_endpoints(void *pvParameters);
//void task_load_endpoints(void *pvParameters);
void func_load_endpoints();
#endif