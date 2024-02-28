#include <inttypes.h>
#include <stdio.h>

#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "protocol_examples_common.h"

#include "ws_client.h"
#include "mqtt_client1.h"

#include "init.h"

static const char *TAG = "MAIN";


void app_main(void)
{
    /* Semaphore for synchronising read and load endpoint processes
    */
    xSemaphore = xSemaphoreCreateBinary();

    ESP_LOGI(TAG, "[APP] Startup..");
    ESP_LOGI(TAG, "[APP] Free memory: %" PRIu32 " bytes", esp_get_free_heap_size());
    ESP_LOGI(TAG, "[APP] IDF version: %s", esp_get_idf_version());


    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
  
    /*set device public idenfier
     *this is the name the outside device identies this device
     *also publishes location data to this topic
     *to avoid conflict with other devices, a truly univerally unique id is required
     *this function sets it to the BASE MAC ADDRESS
     */
    set_device_public_id();

    /*ext1_id is the external node
     * default value: none -> do not track any node
     * any other value means sub to this node id and track
     */
    ext1_id = "none"; 
    
    /*initialize output 
     *i2c display used -> 0.91 OLED display with SSD1309 controller
     */
    init_display();

    /* attempt to get endpoints of broker and server from stdin and write to nvs using oneshot tasks
     * if the user does not input them after 1 min, TIMEOUT ocurrs
     * attempt to read previous ones stored in nvs
     * both tasks self -delete after completing
     */
    xTaskCreate(task_get_endpoints, "get_uri1", configMINIMAL_STACK_SIZE + 1024, NULL, tskIDLE_PRIORITY+10, &task_get_endpoints_xhandle);
    //xTaskCreate(task_load_endpoints, "get_uri2", configMINIMAL_STACK_SIZE + 1024, NULL, tskIDLE_PRIORITY +10 , NULL);
    func_load_endpoints();
    //debug lines 
    //while(1);


    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    /* Start the clients for the first time 
     * mqtt client must be started before ws client
     */
    start_mqtt_client(mqtt_broker_endpoint);
    start_websocket_client(ws_server_endpoint);
}
