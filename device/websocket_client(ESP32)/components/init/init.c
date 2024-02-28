#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_mac.h"

#include "ssd1306.h"

#define ENDPOINTS_INPUT_TIMEOUT 20 * 1000 //20s timeout

static const char *TAG =  "init";

TaskHandle_t task_get_endpoints_xhandle;

char mqtt_broker_endpoint[128];
char ws_server_endpoint[128];

/*external id of this device node
 *set to its mac address
  *mac address needs 6 bytes
*/
char self_id[20];

/*mac address of the external device nodes to track
 *they also publishe their location state under this topic (topic is mac addr)
*/
char ext1_id[20];

SemaphoreHandle_t xSemaphore;
SSD1306_t dev;

static void handle_error_code(esp_err_t error_code) {
    switch (error_code) {
        case ESP_FAIL:
            ESP_LOGI(TAG, "Internal error: ESP_FAIL\n");
            break;
        case ESP_ERR_NVS_NOT_FOUND:
            ESP_LOGI(TAG, "Requested key doesn't exist: ESP_ERR_NVS_NOT_FOUND\n");
            break;
        case ESP_ERR_NVS_INVALID_HANDLE:
            ESP_LOGI(TAG, "Invalid handle or handle closed: ESP_ERR_NVS_INVALID_HANDLE\n");
            break;
        case ESP_ERR_NVS_INVALID_NAME:
            ESP_LOGI(TAG, "Invalid key name: ESP_ERR_NVS_INVALID_NAME\n");
            break;
        case ESP_ERR_NVS_INVALID_LENGTH:
            ESP_LOGI(TAG, "Insufficient length to store data: ESP_ERR_NVS_INVALID_LENGTH\n");
            break;
        default:
            ESP_LOGI(TAG, "Unknown error code\n");
            break;
    }
}
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


static esp_err_t persist_endpoints(){
    nvs_handle_t endpoints_handle;
    esp_err_t err;

    err = nvs_open("init_data", NVS_READWRITE, &endpoints_handle);
    if (err != ESP_OK){
        handle_error_code(err);
        return err;
    }

    err = nvs_set_str(endpoints_handle, "mqtt_endpoint", mqtt_broker_endpoint);
    if (err != ESP_OK) {
        handle_error_code(err);
        return err;
    }

    err = nvs_set_str(endpoints_handle, "ws_endpoint", ws_server_endpoint);
    if (err != ESP_OK) {
        handle_error_code(err);
        return err;
    }

    err = nvs_set_str(endpoints_handle, "ext1_id", ext1_id);
    if (err != ESP_OK) {
        handle_error_code(err);
        return err;
    }

    err = nvs_commit(endpoints_handle);
    if (err != ESP_OK) {
        handle_error_code(err);
        return err;
    }

    nvs_close(endpoints_handle);

    return ESP_OK;
}

static esp_err_t load_endpoints(){
    nvs_handle_t endpoints_handle;
    esp_err_t err;

    err = nvs_open("init_data", NVS_READWRITE, &endpoints_handle);
    if (err != ESP_OK){
        handle_error_code(err);
        return err;
    }

    size_t required_size;

    err = nvs_get_str(endpoints_handle, "mqtt_endpoint", NULL, &required_size);
    if (err != ESP_OK) {
        handle_error_code(err);
        return err;
    }
    err = nvs_get_str(endpoints_handle, "mqtt_endpoint", mqtt_broker_endpoint, &required_size);
    if (err != ESP_OK) {
        handle_error_code(err);
        return err;
    }
    
    err = nvs_get_str(endpoints_handle, "ws_endpoint", NULL, &required_size);
    if (err != ESP_OK) {
        handle_error_code(err);
        return err;
    }
    nvs_get_str(endpoints_handle, "ws_endpoint", ws_server_endpoint, &required_size);
    if (err != ESP_OK) {
        handle_error_code(err);
        return err;
    }

    
    err = nvs_get_str(endpoints_handle, "ext1_id", NULL, &required_size);
    if (err != ESP_OK) {
        handle_error_code(err);
        return err;
    }
    nvs_get_str(endpoints_handle, "ext1_id", ext1_id, &required_size);
    if (err != ESP_OK) {
        handle_error_code(err);
        return err;
    }

    err = nvs_commit(endpoints_handle);
    if (err != ESP_OK) {
        handle_error_code(err);
        return err;
    }

    nvs_close(endpoints_handle);

    return ESP_OK;
}

static esp_err_t get_endpoints(){
    esp_err_t err;

    ESP_LOGI(TAG, "Please enter uri of mqtt broker endpoint");
    get_string(mqtt_broker_endpoint, sizeof(mqtt_broker_endpoint));
    ESP_LOGI(TAG, "Endpoint uri: %s\n", mqtt_broker_endpoint);


    ESP_LOGI(TAG, "Please enter uri of websocket endpoint");
    get_string(ws_server_endpoint, sizeof(ws_server_endpoint));
    ESP_LOGI(TAG, "Endpoint uri: %s\n", ws_server_endpoint);

    ESP_LOGI(TAG, "Please id of node to track (MAC addr)");
    get_string(ext1_id, sizeof(ext1_id));
    ESP_LOGI(TAG, "Track Node MAC: %s\n", ext1_id);

    err = persist_endpoints();
    if(err == ESP_OK){
        ESP_LOGI(TAG, "Endpoints saved to nvs.");
    }

    return err;
}

void init_display(){

    i2c_master_init(&dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
    dev._flip = true;
    ssd1306_init(&dev, 128, 32);
    ssd1306_clear_screen(&dev, false);
	ssd1306_contrast(&dev, 0xff);
    ssd1306_display_text(&dev, 0, "NavSense Beacon", 15, true);
    ssd1306_display_text(&dev, 2, "Booting...", 10, true);
    //NavSense Beacon
}

void set_device_public_id(){
    //set nodes self_id to its mac
    uint8_t mac[6];
    esp_base_mac_addr_get(mac);
    snprintf(self_id, 20, "%02X:%02X:%02X:%02X:%02X:%02X",
            (unsigned)mac[0], (unsigned)mac[1], (unsigned)mac[2], 
            (unsigned)mac[3], (unsigned)mac[4], (unsigned)mac[5]);

    //statically set mac addr of node to be tracked:assumed to be known beforehand
    ext1_id = (char *)"1a2b3c4d5e";

    ESP_LOGI(TAG, "self_id: %s", self_id);
    ESP_LOGI(TAG, "ext1_id: %s", ext1_id);
}

void task_get_endpoints(void *pvParameters) {
    esp_err_t err = get_endpoints();
    if (err != ESP_OK){
        ESP_LOGI(TAG, "Error Occured: %d", err);
    }
    xSemaphoreGive(xSemaphore);

    vTaskDelete(NULL); 

}
/**
void task_load_endpoints(void *pvParameters) {
    esp_err_t err;
    if (xSemaphoreTake(xSemaphore, pdMS_TO_TICKS(ENDPOINTS_INPUT_TIMEOUT)) == pdTRUE) {
        //semaphore taken in time. save read input to nvs. 
        err = persist_endpoints();
        if (err != ESP_OK){
            ESP_LOGI(TAG, "Error Saving Endpoints: %d", err);
        }
    } 
    else {
        //semaphore not taken in time, discard task that reads user input and load endpoints from memory
        vTaskDelete(task_get_endpoints_xhandle);
        ESP_LOGI(TAG, "USER INPUT TIMEOUT OCCURED: Proceeding to Load Endpoints from NVS...");
        err = load_endpoints();
        if (err != ESP_OK){
            ESP_LOGI(TAG, "Error Loading Endpoints: %d", err);
            ESP_LOGI(TAG, "Restarting Device To Re-attempt Load...");
            esp_restart();
        }
        ESP_LOGI(TAG, "mqtt endpoint: %s\n", mqtt_broker_endpoint);
        ESP_LOGI(TAG, "ws endpoint: %s\n", ws_server_endpoint);
    }

    vTaskDelete(NULL); 
}
**/

void func_load_endpoints() {
    esp_err_t err;
    if (xSemaphoreTake(xSemaphore, pdMS_TO_TICKS(ENDPOINTS_INPUT_TIMEOUT)) == pdTRUE) {
        //semaphore taken in time. save read input to nvs. 
        err = persist_endpoints();
        if (err != ESP_OK){
            ESP_LOGI(TAG, "Error Saving Endpoints: %d", err);
        }
    } 
    else {
        //semaphore not taken in time, discard task that reads user input and load endpoints from memory
        vTaskDelete(task_get_endpoints_xhandle);
        ESP_LOGI(TAG, "USER INPUT TIMEOUT OCCURED: Proceeding to Load Endpoints from NVS...");
        err = load_endpoints();
        if (err != ESP_OK){
            ESP_LOGI(TAG, "Error Loading Endpoints: %d", err);
            ESP_LOGI(TAG, "Restarting Device To Re-attempt Load...");
            esp_restart();
        }
        ESP_LOGI(TAG, "mqtt endpoint: %s\n", mqtt_broker_endpoint);
        ESP_LOGI(TAG, "ws endpoint: %s\n", ws_server_endpoint);
    }

}