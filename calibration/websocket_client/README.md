# Websocket server
The example starts a websocket server on a local network. You need a websocket client to interact with the server. The `websocket_client` in the root directory can be used for this interaction. 

## How to Use

### Dependencies
- python 3(Development used 3.12)
- websocket python library

### Set the websocket server address

```
ws = websocket.WebSocketApp('ws://ws_server_ip_addr/ws',
                                on_open=on_open,
                                on_message=on_message,
                                on_error=on_error,
                                on_close=on_close)
```

See the README in `websocket_server(ESP32)` on how to find out `ws_server_ip_addr`


## Scanning For WAPs

When the client is connected to the server, a text input field is presented to the user. 

* To scan for WAPs, the websocket client must send "findAps" to the server. Upon reciept of the of this string, It starts scanning for WAPs. The scan takes about 1 - 2 mins to complete. The data is sent back to the client. 

### Format of the Scan Output Sent to the client
| BSSID | SSID | ENCRYPTION| CHANNEL | RSSI | NO OF OCCURENCES |

* To write the data into database, the user must enter "save location" in the text prompt. Location is the tag of the fingerprint Upon reciept of this, the fingerprint of the location is constructed and saved
## Example Usage

```
Opened connection
Enter message to send (or type 'exit' to close the connection): findAps
Enter message to send (or type 'exit' to close the connection): Received message: 88B1E1974121, -63, 17
88B0E1974120, -63, 17
88B0E19726C0, -72, 17
88B0E19726C1, -72, 18
6C8D075D98A3, -76, 20
6C8D775098A4, -75, 20
88B1E199C301, -76, 12
6C8D775098A2, -75, 20
88B1E1982201, -78, 18
88B1E10D2200, -79, 17
88B1E1866621, -82, 15
88B1E1066620, -82, 14
88B1E18D12E1, -83, 18
F01D2D374563, -83, 20
F01D2D774562, -83, 20
88B1E12D28E1, -83, 19
88B1E10D12E0, -84, 13
88B1E10D28E0, -83, 15
88B1E1904580, -84, 13
88B1E1904581, -84, 13
88B1E04B78C1, -87, 6
F01D2D077AE3, -91, 2
88B1E999C300, -76, 11
88B1E04B78C0, -88, 4
F01D28E780E2, -88, 3
F01D2EE780E3, -88, 1
F01D2D077AE2, -91, 1
```