#include <stdio.h>
#include "esp_wifi.h"
#include "esp_system.h"

#include "protocol_examples_common.h"

#include "freertos/semphr.h"

#include "esp_log.h"
#include "esp_event.h"

#include "scan.h"
#include "esp_websocket_client.h"

#include "ws_client.h"
#include "mqtt_client1.h"

#include "init.h"

static const char *TAG = "ws_client";


static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}


#if CONFIG_WEBSOCKET_URI_FROM_STDIN
static void get_string(char *line, size_t size)
{
    int count = 0;
    while (count < size) {
        int c = fgetc(stdin);
        if (c == '\n') {
            line[count] = '\0';
            break;
        } else if (c > 0 && c < 127) {
            line[count] = c;
            ++count;
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

#endif /* CONFIG_WEBSOCKET_URI_FROM_STDIN */

static void websocket_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_websocket_event_data_t *data = (esp_websocket_event_data_t *)event_data;
    switch (event_id) {
    case WEBSOCKET_EVENT_CONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_CONNECTED");
        break;
    case WEBSOCKET_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DISCONNECTED");
        /**
        esp_err_t status = ESP_FAIL;
        while(status != ESP_OK){
            ESP_LOGI(TAG, "Attempting server reconnect...");
            status = esp_websocket_client_start(data->client);
            vTaskDelay(9000/portTICK_PERIOD_MS);
            }
        **/
        /**
        log_error_if_nonzero("HTTP status code",  data->error_handle.esp_ws_handshake_status_code);
        if (data->error_handle.error_type == WEBSOCKET_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", data->error_handle.esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", data->error_handle.esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  data->error_handle.esp_transport_sock_errno);
        }
        **/
        break;
    case WEBSOCKET_EVENT_DATA:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_DATA");
        ESP_LOGI(TAG, "Received opcode=%d", data->op_code);
        if (data->op_code == 0x08 && data->data_len == 2) {
            ESP_LOGW(TAG, "Received closed message with code=%d", 256 * data->data_ptr[0] + data->data_ptr[1]);
        } else {
            ESP_LOGW(TAG, "Received=%.*s", data->data_len, (char *)data->data_ptr);
        }

        //Code to send recieved location of current node to output device

        //strcat(node1_prompt, (char *)data->data_ptr);
        ssd1306_clear_line(&dev, 2, false);
        ssd1306_display_text(&dev, 2, (char *)data->data_ptr, 6, false);

        
        //publish this nodes location to broker, under its public id
        publish((char *)self_id, (char *)data->data_ptr);

        break;
    case WEBSOCKET_EVENT_ERROR:
        ESP_LOGI(TAG, "WEBSOCKET_EVENT_ERROR");
        log_error_if_nonzero("HTTP status code",  data->error_handle.esp_ws_handshake_status_code);
        if (data->error_handle.error_type == WEBSOCKET_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", data->error_handle.esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", data->error_handle.esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  data->error_handle.esp_transport_sock_errno);
        }
        break;
    }
}

void start_websocket_client(char *endpoint)
{
    ssd1306_display_text(&dev, 3, "int tr setup...", 15, true);

    ESP_LOGI(TAG, "Endpoint uri: %s\n", endpoint);

    esp_websocket_client_config_t websocket_cfg = {};
    websocket_cfg.uri = endpoint;
    websocket_cfg.network_timeout_ms = 20 * 1000; //Network timeout 20s
    websocket_cfg.reconnect_timeout_ms = 3 * 1000; //reconnect timeout 3s
    

    ESP_LOGI(TAG, "Connecting to %s...", websocket_cfg.uri);

    esp_websocket_client_handle_t ws_client = esp_websocket_client_init(&websocket_cfg);
    esp_websocket_register_events(ws_client, WEBSOCKET_EVENT_ANY, websocket_event_handler, (void *)ws_client);
 
    esp_err_t status = ESP_FAIL;
    while(status != ESP_OK){
        ESP_LOGI(TAG, "Attempting server connect...");
        status = esp_websocket_client_start(ws_client);
    }


    char *scan_results_buf = NULL;
    //uint16_t max_scans = 20;
    uint16_t verbose = 0; 
    uint16_t nbuf = 0; 

    while(1){
        ESP_LOGI(TAG, "Scanning for WAPs...");  
        wifi_scan(&scan_results_buf, &nbuf, verbose);
        //wifi_scan2(&scan_results_buf, &nbuf, verbose, max_scans);

        if(esp_websocket_client_is_connected(ws_client)){
            ESP_LOGI(TAG, "Sending Results to Server...");
            esp_websocket_client_send_text(ws_client, scan_results_buf, nbuf, portMAX_DELAY);
        
        } else{
            ESP_LOGI(TAG, "Disconnected From Server");
        }

        //free scan_results_buf
        free(scan_results_buf);
        vTaskDelay(2000/portTICK_PERIOD_MS);
    }

}