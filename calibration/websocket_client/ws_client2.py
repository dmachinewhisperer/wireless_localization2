import process_aps
import websocket
import warnings
import _thread
import sqlite3
import time
import sys

# Suppress all warnings
warnings.filterwarnings("ignore")

index = 1

location_tag = None

def on_message(ws, message):
    global location_tag, index
    print(f"Received message: index = {index}\n {message}")

    index = index + 1

    db = "devdb.db"
    connection = sqlite3.connect(db)
    process_aps.process_wss_res(message)
    process_aps.update_fv(connection)
    process_aps.construct_fingerprint(connection, location_tag)
    connection.close()
    sys.stdout.flush()
   
def on_error(ws, error):
    print(f"Error occurred: {error}")

def on_close(ws, close_status_code, close_msg):
    print(f"Connection closed with status code {close_status_code}: {close_msg}")

def on_open(ws):
    print("Opened connection")
    ws.send("findApsAuto")
    print("Sent trigger string findApsAuto")


if __name__ == "__main__":

    websocket.enableTrace(False)

    esp32_ipv4_ip_addr = ''
    location_tag = ''

    while esp32_ipv4_ip_addr =='':
        esp32_ipv4_ip_addr = input("Enter WSS addr: ")
    
    while location_tag == '':
        location_tag = input("Enter location tag: ")
    
    ws = websocket.WebSocketApp('ws://' + esp32_ipv4_ip_addr + ':80/ws',
                                on_open=on_open,
                                on_message=on_message,
                                on_error=on_error,
                                on_close=on_close)

    # Run the WebSocket connection in the main thread
    ws.run_forever()

    print("WebSocket connection closed.")
