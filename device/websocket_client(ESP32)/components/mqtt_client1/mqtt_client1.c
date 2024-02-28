#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"

#include "esp_event.h"
#include "protocol_examples_common.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "mqtt_client1.h"
#include "init.h"

static const char *TAG = "MQTT_CLIENT";

esp_mqtt_client_handle_t client;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

#if CONFIG_BROKER_URL_FROM_STDIN
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

#endif /* CONFIG_BROKER_URL_FROM_STDIN */

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        //once connected, subscribe to get location updates of tracked nodes
        if(strcmp(ext1_id, "none") == 0){
            ESP_LOGI(TAG, "No node to track. ext1_id is set to none");
            ssd1306_clear_line(&dev, 3, false);
            ssd1306_display_text(&dev, 3, "no ext node", 12, false);
        }
        else{
            subscribe(ext1_id);
        }
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);

        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);

        //Code to send recieved location to of tracked node to output device

        //strcat(node2_prompt, (char *)event->data);
        ssd1306_clear_line(&dev, 3, false);
        ssd1306_display_text(&dev, 3, (char *)event->data, 6, false);

        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}


void publish(char *topic, char *payload)
{
    int msg_id;
    msg_id = esp_mqtt_client_publish(client, topic, payload, 0, 1, 0);
    if(msg_id > 0){
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
    } else{
        ESP_LOGE(TAG, "publish unsuccessful");
    }

}

static void subscribe(char *topic)
{
    int msg_id;
    msg_id = esp_mqtt_client_subscribe(client, topic, 0);
    if(msg_id > 0){
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
    } else{
        ESP_LOGE(TAG, "subscribe unsuccessful");
    }
}

static void unsubscribe(char *topic)
{
    int msg_id;;
    msg_id = esp_mqtt_client_unsubscribe(client, topic);

    if(msg_id > 0){
        ESP_LOGI(TAG, "sent unsubscribe successful, msg_id=%d", msg_id);
    }else{
        ESP_LOGE(TAG, "unsubscribe unsuccessful");
    }
    
}

void start_mqtt_client(char *endpoint)
{
    ssd1306_display_text(&dev, 2, "ext tr setup...", 15, true);
    
    ESP_LOGI(TAG, "MQTT Broker Endpoint: %s\n", endpoint);

    esp_mqtt_client_config_t mqtt_cfg = {};
    mqtt_cfg.broker.address.uri = endpoint;

    client = esp_mqtt_client_init(&mqtt_cfg);

    /* The last argument may be used to pass data to the event handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}