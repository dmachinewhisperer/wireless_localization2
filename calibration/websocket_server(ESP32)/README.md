# Websocket server
The application starts a websocket server on a local network. You need a websocket client to interact with the server. The `websocket_client` in the root directory can be used for this interaction. 

## How to Use

### Hardware Required

This example can be executed on any ESP32 development board, the only required interface is WiFi connection to a local network. This project was tested on an ESP32-WROOM-32 board. 

### Dependencies
See the CMakeLists.txt in the component and main directories

### Configure the project

* Open the project configuration menu (`idf.py menuconfig`)
* Configure Wi-Fi under "Example Connection Configuration" menu. 

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
idf.py -p PORT flash monitor
```

(To exit the serial monitor, type ``Ctrl-T-X``.)

See the ESP-IDF  Guide for full steps to configure and use ESP-IDF to build projects.

## Scanning For WAPs

* To scan for WAPs, the websocket_client must send a trigger_string to the server. Upon reciept of the of this string, It starts scanning for WAPs. 
* To handle signal fluctuations, the scan is carried out 20 times(can be modfied in the `wifi_scan` using the `max_scan` variable in the scan component).
* To be included in the final result, a WAP must show up at least 75% of the number of times the scan is carried out.(can be modifed in the `wifi_scan` using the `thresh` variable)
* The RSSIs is averaged and sent back to the client after the scan completes. 

### Trigger Strings
Two trigger strings are defined:

* `FindApsAuto` : Upon receipt of this string, the server initiates an indefinite WAP scan in a loop. Results are sent to the the websocket client as soon as a scan completes. The client must make arrangements to persist this information in a database. 
```Note that the server cannot service any subsequent trigger string once it enters this loop. ```

* `FindApsManu` : 20 scans are carried out consecutively, and RSSIs averaged. Then sent to the client.


### Format of the Scan Output Sent to the client
| BSSID | SSID | ENCRYPTION| CHANNEL | RSSI | NO OF OCCURENCES |


## Notes
- If your LAN uses DHCP to assign IP addresses, the IP address of the ESP32 may be different between power cycles. Make sure to copy the current IP address from the serial monitor after startup to the websocket client. Find the following lines in the serial monitor output:

```
I (6518) esp_netif_handlers: example_netif_sta ip: 172.22.194.205, mask: 255.255.240.0, gw: 172.22.192.1
I (6518) example_connect: Got IPv4 event: Interface "example_netif_sta" address: 172.22.194.205
I (6588) example_connect: Got IPv6 event: Interface "example_netif_sta" address: fe80:0000:0000:0000:b2b2:1cff:fea7:4080, type: ESP_IP6_ADDR_IS_LINK_LOCAL

```

- You may need to increase `DEFAULT_SCAN_LIST_SIZE` in scan.c based on the number of WAPs you expect to show up in each location. 

## Example Output
```
I (788) example_connect: Connecting to GITAM...
I (788) example_connect: Waiting for IP(s)
I (3198) wifi:new:<11,0>, old:<1,0>, ap:<255,255>, sta:<11,0>, prof:1
I (4938) wifi:state: init -> auth (b0)
I (4948) wifi:state: auth -> assoc (0)
I (4968) wifi:state: assoc -> run (10)
I (5008) wifi:connected with GITAM, aid = 3, channel 11, BW20, bssid = 6c:8d:77:5d:98:a3
I (5008) wifi:security: WPA2-PSK, phy: bgn, rssi: -80
I (5008) wifi:pm start, type: 1

I (5028) wifi:AP's beacon interval = 102400 us, DTIM period = 1
I (5528) wifi:<ba-add>idx:0 (ifx:0, 6c:8d:77:5d:98:a3), tid:6, ssn:2, winSize:64
I (5548) wifi:<ba-del>idx:0, tid:6
I (5548) wifi:<ba-add>idx:0 (ifx:0, 6c:8d:77:5d:98:a3), tid:6, ssn:3, winSize:64
I (6518) esp_netif_handlers: example_netif_sta ip: 172.22.194.205, mask: 255.255.240.0, gw: 172.22.192.1
I (6518) example_connect: Got IPv4 event: Interface "example_netif_sta" address: 172.22.194.205
I (6588) example_connect: Got IPv6 event: Interface "example_netif_sta" address: fe80:0000:0000:0000:b2b2:1cff:fea7:4080, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6588) example_common: Connected to example_netif_sta
I (6598) example_common: - IPv4 address: 172.22.194.205,
I (6598) example_common: - IPv6 address: fe80:0000:0000:0000:b2b2:1cff:fea7:4080, type: ESP_IP6_ADDR_IS_LINK_LOCAL
I (6608) ws_server: Starting server on port: '80'
I (6618) ws_server: Registering URI handlers
I (6618) main_task: Returned from app_main()
I (363958) wifi:<ba-add>idx:1 (ifx:0, 6c:8d:77:5d:98:a3), tid:0, ssn:0, winSize:64
I (364038) ws_server: Handshake done, the new connection was opened
I (399998) ws_server: frame len is 7
I (399998) ws_server: Got packet with message: findApsManu
I (399998) ws_server: Packet type: 1
I (400008) ws_server: Find APs Request Acknowleged. Initiating Scan...
I (402918) scan: iter 0: Total APs scanned = 25
I (402918) scan: Processing APs...
I (402918) scan: Iter 0 done
I (405818) scan: iter 1: Total APs scanned = 19
I (405818) scan: Processing APs...
I (405818) scan: Iter 1 done
I (408718) scan: iter 2: Total APs scanned = 20
I (408718) scan: Processing APs...
I (408718) scan: Iter 2 done
I (411618) scan: iter 3: Total APs scanned = 19
I (411618) scan: Processing APs...
I (411618) scan: Iter 3 done
I (414518) scan: iter 4: Total APs scanned = 18
I (414518) scan: Processing APs...
I (414518) scan: Iter 4 done
I (417418) scan: iter 5: Total APs scanned = 19
I (417418) scan: Processing APs...
I (417418) scan: Iter 5 done
I (420318) scan: iter 6: Total APs scanned = 24
I (420318) scan: Processing APs...
I (420318) scan: Iter 6 done
I (423218) scan: iter 7: Total APs scanned = 20
I (423218) scan: Processing APs...
I (423218) scan: Iter 7 done
I (426118) scan: iter 8: Total APs scanned = 18
I (426118) scan: Processing APs...
I (426118) scan: Iter 8 done
I (429018) scan: iter 9: Total APs scanned = 19
I (429018) scan: Processing APs...
I (429018) scan: Iter 9 done
I (431918) scan: iter 10: Total APs scanned = 22
I (431918) scan: Processing APs...
I (431918) scan: Iter 10 done
I (434818) scan: iter 11: Total APs scanned = 21
I (434818) scan: Processing APs...
I (434818) scan: Iter 11 done
I (437718) scan: iter 12: Total APs scanned = 18
I (437718) scan: Processing APs...
I (437718) scan: Iter 12 done
I (440618) scan: iter 13: Total APs scanned = 21
I (440618) scan: Processing APs...
I (440618) scan: Iter 13 done
I (443518) scan: iter 14: Total APs scanned = 21
I (443518) scan: Processing APs...
I (443518) scan: Iter 14 done
I (446418) scan: iter 15: Total APs scanned = 23
I (446418) scan: Processing APs...
I (446418) scan: Iter 15 done
I (449318) scan: iter 16: Total APs scanned = 19
I (449318) scan: Processing APs...
I (449318) scan: Iter 16 done
I (452218) scan: iter 17: Total APs scanned = 22
I (452218) scan: Processing APs...
I (452218) scan: Iter 17 done
I (455118) scan: iter 18: Total APs scanned = 17
I (455118) scan: Processing APs...
I (455118) scan: Iter 18 done
I (458018) scan: iter 19: Total APs scanned = 20
I (458018) scan: Processing APs...
I (458018) scan: Iter 19 done
I (458018) scan: Scan code exit...
I (458028) ws_server: Send Complete

I (458028) ws_server:
88B1E1974120, -62, 17
88B1E1974121, -63, 17
88B1E199C301, -78, 15
88B1E199C300, -77, 17
6C8D775D98A4, -77, 20
6C8D775D98A3, -77, 20
88B1E19726C1, -71, 17
88B1E19726C0, -71, 19
6C8D775D98A2, -77, 20
88B1E19D28E1, -82, 18
88B1E19D2200, -76, 19
88B1E19D2201, -76, 20
88B1E19D28E0, -83, 16
88B1E1466620, -83, 17
88B1E19D12E1, -84, 14
88B1E1466621, -82, 17
F01D2DE74563, -86, 12
88B1E19D12E0, -83, 18
88B1E1974581, -85, 19
F01D2DE74562, -88, 10
88B1E1974580, -85, 12
88B1E14B78C1, -88, 1
F01D2DE77AE2, -92, 4
88B1E146CE01, -91, 2
F01D2DE77AE3, -92, 4
88B1E14B6E81, -92, 1
F01D2DE780E3, -90, 2
```
