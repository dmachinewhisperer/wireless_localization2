#include <esp_wifi.h>
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"
#include "protocol_examples_common.h"

#include <esp_http_server.h>

#include "scan.h"

static const char *TAG = "ws_server";

/*
 * Structure holding server handle
 * and internal socket fd in order
 * to use out of request send
 */
struct async_resp_arg {
    httpd_handle_t hd;
    int fd;
};

/**
 * This function is used for fragmenting large payloads.
 * The websocket implmentation does not handle this automatically so has to be done manually
 * Sets the FIN bit for the final chunk sent to the client. 
*/
static esp_err_t send_chunks(char * payload, int payloadSize, int chunkSize, 
    httpd_ws_frame_t * ws_pkt,httpd_req_t *req){

        esp_err_t ret = !ESP_OK; 
        for (size_t offset = 0; offset < payloadSize; offset += chunkSize) {
        size_t remaining = payloadSize - offset;
        size_t sendSize = (remaining < chunkSize) ? remaining : chunkSize;

        //memset(&chunk_pkt, 0, sizeof(httpd_ws_frame_t));
        ws_pkt->type = HTTPD_WS_TYPE_TEXT;
        ws_pkt->payload = (uint8_t *)(payload + offset);
        ws_pkt->len = sendSize;
        ws_pkt->final = (offset + sendSize >= payloadSize);

        ret = httpd_ws_send_frame(req, ws_pkt);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
            break;
        }
    }
    return ESP_OK;

}
/**
 * This function is similar to send_chunks except that it sets the final bit 
 * when sending the last block of data containing multiple \0, controlled using sendFinal val
*/
static esp_err_t send_chunks1(char * payload, int payloadSize, int chunkSize, int setFinal,
    httpd_ws_frame_t * ws_pkt,httpd_req_t *req){
        esp_err_t ret = !ESP_OK; 

        if(payload == NULL){
            ESP_LOGI(TAG,"payload is NULL");
            return ret;
        }

        for (size_t offset = 0; offset < payloadSize; offset += chunkSize) {
        size_t remaining = payloadSize - offset;
        size_t sendSize = (remaining < chunkSize) ? remaining : chunkSize;

        //memset(&chunk_pkt, 0, sizeof(httpd_ws_frame_t));
        ws_pkt->type = HTTPD_WS_TYPE_TEXT;
        ws_pkt->payload = (uint8_t *)(payload + offset);
        ws_pkt->len = sendSize;
        if(setFinal){
            ws_pkt->final = (offset + sendSize >= payloadSize);

        } else{
            ws_pkt->final = 0;
        }
        
        ret = httpd_ws_send_frame(req, ws_pkt);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "httpd_ws_send_frame failed with %d", ret);
            break;
        }
    }
    return ESP_OK;

}
/*
 * This handler sends back the sensed APs to the requesting client
 */
static esp_err_t controller_handler(httpd_req_t *req)
{

    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "Handshake done, the new connection was opened");
        return ESP_OK;
    }
    httpd_ws_frame_t ws_pkt;
    uint8_t *buf = NULL;
    memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    /* Set max_len = 0 to get the frame len */
    esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "httpd_ws_recv_frame failed to get frame len with %d", ret);
        return ret;
    }
    ESP_LOGI(TAG, "frame len is %d", ws_pkt.len);
    if (ws_pkt.len) {
        /* ws_pkt.len + 1 is for NULL termination as we are expecting a string */
        buf = calloc(1, ws_pkt.len + 1);
        if (buf == NULL) {
            ESP_LOGE(TAG, "Failed to calloc memory for buf");
            return ESP_ERR_NO_MEM;
        }
        ws_pkt.payload = buf;
        /* Set max_len = ws_pkt.len to get the frame payload */
        ret = httpd_ws_recv_frame(req, &ws_pkt, ws_pkt.len);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "httpd_ws_recv_frame failed with %d", ret);
            free(buf);
            return ret;
        }
        ESP_LOGI(TAG, "Got packet with message: %s", ws_pkt.payload);
    }

    ESP_LOGI(TAG, "Packet type: %d", ws_pkt.type);

    if (ws_pkt.type == HTTPD_WS_TYPE_TEXT &&
        (strcmp((char*)ws_pkt.payload,"findApsAuto") == 0 || 
        strcmp((char*)ws_pkt.payload,"findApsManu") == 0) )  {

        char *scan_results_buf = NULL;
        uint16_t max_scans = 20;
        uint16_t nth_scan = 0;
        uint16_t verbose = 0; 
        uint16_t nbuf = 0; 


        ESP_LOGI(TAG, "Find APs Request Acknowleged. Initiating Scan...");  
        if (strcmp((char*)ws_pkt.payload,"findApsAuto") == 0) {
            //continuosly scan and send WAP results
            while(1){
                wifi_scan(&scan_results_buf, &nbuf, verbose);

                ws_pkt.type = HTTPD_WS_TYPE_TEXT;
                ws_pkt.payload = (uint16_t *)scan_results_buf;
                ws_pkt.len = nbuf;
                ret = httpd_ws_send_frame(req, &ws_pkt);
                
                free(scan_results_buf);

                if(ret != ESP_OK){
                    ESP_LOGI(TAG, "Send failed. Reconnect client");
                    return !ESP_OK;
                }
                ESP_LOGI(TAG, "Sent scan results %d\n", ++nth_scan);
            }
            
        } else{
            wifi_scan2(&scan_results_buf, &nbuf, verbose, max_scans);
        }
         
       //send back scan data to websocket client
        ws_pkt.type = HTTPD_WS_TYPE_TEXT;
        ws_pkt.payload = (uint16_t *)scan_results_buf;
        ws_pkt.len = nbuf;
        ret = httpd_ws_send_frame(req, &ws_pkt);
        
        ESP_LOGI(TAG, "Send Complete\n");

        ESP_LOGI(TAG, "\n%s", scan_results_buf);

        free(scan_results_buf);
    }

    free(buf);
    return ESP_OK;
}

static const httpd_uri_t ws = {
        .uri        = "/ws",
        .method     = HTTP_GET,
        .handler    = controller_handler,
        .user_ctx   = NULL,
        .is_websocket = true
};


httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    /**
     * the params used to configure confi is defined in esp_http_server.h
     * config.stack_size is set to 4096
     * Sometimes the webserver raises a stackoverflow error running at this stack_size. 
    */
    config.stack_size = 8192;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Registering the ws handler
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &ws);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static esp_err_t stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}

void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        if (stop_webserver(*server) == ESP_OK) {
            *server = NULL;
        } else {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
    }
}

void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}


