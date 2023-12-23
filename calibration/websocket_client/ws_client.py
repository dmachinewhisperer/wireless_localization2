import process_aps
import websocket
import warnings
import _thread
import sqlite3
import time
import sys

# Suppress all warnings
warnings.filterwarnings("ignore")

# Lock to control thread pausing and resuming
thread_lock = _thread.allocate_lock()
pause_thread = False
exception = None; 

wss_response = None

def on_message(ws, message):
    global wss_response

    print(f"Received message: {message}")
    wss_response = message; 
   
def on_error(ws, error):
    print(f"Error occurred: {error}")

def on_close(ws, close_status_code, close_msg):
    print(f"Connection closed with status code {close_status_code}: {close_msg}")

def on_open(ws):
    print("Opened connection")

    # Start a new thread for user input
    _thread.start_new_thread(send_user_input, (ws,))

def send_user_input(ws):
    global wss_response
    
    global pause_thread
    while True:
        if wss_response is not None:
            db = "devdb.db"
            connection = sqlite3.connect(db)
            process_aps.process_wss_res(wss_response)
            process_aps.update_fv(connection)
            process_aps.construct_fingerprint(connection)
            connection.close()
            wss_response = None

        user_input = input("Enter message to send (or type 'exit' quit): ")
        if user_input.lower() == 'exit':
            ws.close()
            break
        else:
            if user_input == '':
                continue
            ws.send(user_input)

if __name__ == "__main__":
    websocket.enableTrace(False)
    esp32_ipv4_ip_addr = ''
    while esp32_ipv4_ip_addr =='':
        esp32_ipv4_ip_addr = input("Enter WSS addr: ")

    ws = websocket.WebSocketApp('ws://' + esp32_ipv4_ip_addr + ':80/ws',
                                on_open=on_open,
                                on_message=on_message,
                                on_error=on_error,
                                on_close=on_close)

    # Run the WebSocket connection in the main thread
    ws.run_forever()

    print("WebSocket connection closed.")
