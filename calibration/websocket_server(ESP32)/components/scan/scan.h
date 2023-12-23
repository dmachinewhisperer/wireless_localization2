#ifndef SCAN_H
#define SCAN_H

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"


void wifi_scan(char **scan_results_buf, uint16_t *nbuf, uint16_t verbose);

void wifi_scan2(char **scan_results_buf, uint16_t *nbuf, uint16_t verbose, uint16_t max_scans);


#endif