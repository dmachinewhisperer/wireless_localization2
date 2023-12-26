#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"

//#define DEFAULT_SCAN_LIST_SIZE CONFIG_EXAMPLE_SCAN_LIST_SIZE
#define DEFAULT_SCAN_LIST_SIZE 50
#define MAX_ENTRY_LENGTH 64


static const char *TAG = "scan";


/* Initialize Wi-Fi as sta and set scan method */
//The params are the output of the func. 
void wifi_scan(char **scan_results_buf, uint16_t *nbuf, uint16_t verbose)
{

    char buffer[MAX_ENTRY_LENGTH];
    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    uint16_t bytes_written = 0; 
    uint16_t curr_mem_loc  = 0;

    memset(ap_info, 0, sizeof(ap_info));
    esp_wifi_scan_start(NULL, true);
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    ESP_LOGI(TAG, "Total APs scanned = %u", ap_count);
    
    *scan_results_buf = (char *)malloc(MAX_ENTRY_LENGTH * ap_count * sizeof(char));
    if(*scan_results_buf == NULL){
        ESP_LOGE(TAG, "Failed to malloc memory for scan entries");
        return;
    }


    //Package scan results for transmission. 
    for (int i = 0; i < ap_count; i++) {  
        /**
        bytes_written = snprintf(buffer, MAX_ENTRY_LENGTH, "%02X%02X%02X%02X%02X%02X, %s, %d, %d, %d", 
                    ap_info[i].bssid[0],ap_info[i].bssid[1],ap_info[i].bssid[2],
                    ap_info[i].bssid[3],ap_info[i].bssid[4],ap_info[i].bssid[5],
                    ap_info[i].ssid, ap_info[i].rssi, ap_info[i].authmode, ap_info[i].primary) + 1;

        **/

        bytes_written = snprintf(buffer, MAX_ENTRY_LENGTH, "%02X%02X%02X%02X%02X%02X, %d, %d", 
                    ap_info[i].bssid[0],ap_info[i].bssid[1],ap_info[i].bssid[2],
                    ap_info[i].bssid[3],ap_info[i].bssid[4],ap_info[i].bssid[5],
                    ap_info[i].rssi, 1) + 1;

        strcpy(*scan_results_buf + curr_mem_loc, buffer);

        curr_mem_loc = curr_mem_loc + bytes_written;

        //Replace \0 introuduced by snprintf with delimiters
        if(i != (ap_count - 1)){
            *(*scan_results_buf + curr_mem_loc - 1) = '\n';
        }

        //ESP_LOGI(TAG, "curr_mem_loc: %d",curr_mem_loc);

    }

    if (verbose){  
            ESP_LOGI(TAG, "Total APS: %d",ap_count);
           ESP_LOGI(TAG, "WiFi Scan Results: \n%s", *scan_results_buf);
        }

    *nbuf = curr_mem_loc - 1;
    ESP_LOGI(TAG, "Scan code exit...");

}

/**
 * Initiates scan n times and returns the averaged rssi to reduce signal fluctutation
*/
void wifi_scan2(char **scan_results_buf, uint16_t *nbuf, uint16_t verbose, uint16_t max_scans)
{

    char buffer[MAX_ENTRY_LENGTH];
    uint16_t number = DEFAULT_SCAN_LIST_SIZE;
    wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    uint16_t ap_count = 0;
    uint16_t bytes_written = 0; 
    uint16_t curr_mem_loc  = 0;

    //uint16_t max_scans = 20;
    uint16_t thresh = (uint16_t)(0.75 * max_scans); 
    struct ap_record{
        uint8_t n_occ;
        uint8_t bssid[6];
        int16_t rssi;
    };

    struct ap_record ap_table[DEFAULT_SCAN_LIST_SIZE] = {{0}};
    
    uint8_t nth_entry = 0;      //track no records in ap_table
    uint8_t bssid_exists = 0; 
    //memset(ap_info, 0, sizeof(ap_info));
    for(int i = 0; i<max_scans; i++){

        esp_wifi_scan_start(NULL, true);
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
        ESP_LOGI(TAG, "iter %d: Total APs scanned = %u", i, ap_count);
        ESP_LOGI(TAG, "Processing APs...");

        for(int j = 0; j<ap_count; j++){
            bssid_exists = 0; 
            for(int k = 0; k < nth_entry; k++){
                //if ap_record exists in table, accumulate the rssi

                if( ap_info[j].bssid[0] == ap_table[k].bssid[0] &&
                    ap_info[j].bssid[1] == ap_table[k].bssid[1] &&
                    ap_info[j].bssid[2] == ap_table[k].bssid[2] &&
                    ap_info[j].bssid[3] == ap_table[k].bssid[3] &&
                    ap_info[j].bssid[4] == ap_table[k].bssid[4] && 
                    ap_info[j].bssid[5] == ap_table[k].bssid[5]  )
                {
                    ap_table[k].n_occ +=1;
                    ap_table[k].rssi = ap_table[k].rssi + ap_info[j].rssi; 
                    bssid_exists = 1; 
                    break; 

                }
            }
            //if ap_record is not found, make a new entry
            if(!bssid_exists){
                ap_table[nth_entry].bssid[0] = ap_info[j].bssid[0];
                ap_table[nth_entry].bssid[1] = ap_info[j].bssid[1];
                ap_table[nth_entry].bssid[2] = ap_info[j].bssid[2];
                ap_table[nth_entry].bssid[3] = ap_info[j].bssid[3];
                ap_table[nth_entry].bssid[4] = ap_info[j].bssid[4];
                ap_table[nth_entry].bssid[5] = ap_info[j].bssid[5];

                ap_table[nth_entry].rssi = ap_info[j].rssi;
                ap_table[nth_entry].n_occ = 1;
                nth_entry +=1;

                if(nth_entry == DEFAULT_SCAN_LIST_SIZE){
                    ESP_LOGE(TAG,"Table out of memory");
                    return; 
                }
                
            }
        }
        //vTaskDelay(pdMS_TO_TICKS(2000));
        ESP_LOGI(TAG, "Iter %d done",i);
    }

    *scan_results_buf = (char *)malloc(MAX_ENTRY_LENGTH * nth_entry * sizeof(char));
    if(*scan_results_buf == NULL){
        ESP_LOGE(TAG, "Failed to malloc memory for scan entries");
        return;
    }

    //Package scan results for transmission. 
    for (int i = 0; i < nth_entry; i++) {  

        /**
         * each entry must show up at least 75% times, otherwise, drop the record
        
       if(ap_table[i].n_occ < thresh){
        ESP_LOGI(TAG, "One scan record dropped");
        continue;
       }
       **/

        //average rssis 
        ap_table[i].rssi = (uint16_t)(ap_table[i].rssi/ap_table[i].n_occ);

        bytes_written = snprintf(buffer, MAX_ENTRY_LENGTH, "%02X%02X%02X%02X%02X%02X, %d, %d", 
                    ap_table[i].bssid[0],ap_table[i].bssid[1],ap_table[i].bssid[2],
                    ap_table[i].bssid[3],ap_table[i].bssid[4],ap_table[i].bssid[5],
                    ap_table[i].rssi, ap_table[i].n_occ) + 1;

 
        strcpy(*scan_results_buf + curr_mem_loc, buffer);

        curr_mem_loc = curr_mem_loc + bytes_written;

        //Replace \0 introuduced by snprintf with delimiters
        if(i != (nth_entry - 1)){
            *(*scan_results_buf + curr_mem_loc - 1) = '\n';
        }

        //ESP_LOGI(TAG, "curr_mem_loc: %d",curr_mem_loc);
    }
    if (verbose){  
        ESP_LOGI(TAG, "Total APS: %d",nth_entry);
        ESP_LOGI(TAG, "WiFi Scan Results: \n%s", *scan_results_buf);
        }
    *nbuf = curr_mem_loc - 1;
    ESP_LOGI(TAG, "Scan code exit...");

}
