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


def on_message(ws, message):

    print(f"Received message: {message}")

    db = "devdb.db"
    connection = sqlite3.connect(db)
    process_aps.process_wss_res(message)
    process_aps.update_fv(connection)
    process_aps.construct_fingerprint(connection)

    connection.close()

    global pause_thread
    pause_thread = True

def on_error(ws, error):
    print(f"Error occurred: {error}")

def on_close(ws, close_status_code, close_msg):
    print(f"Connection closed with status code {close_status_code}: {close_msg}")

def on_open(ws):
    print("Opened connection")

    # Start a new thread for user input
    _thread.start_new_thread(send_user_input, (ws,))

def send_user_input(ws):
    global pause_thread
    while True:
        with thread_lock:
            if pause_thread:
                pause_thread = False
                _thread.exit() 

        user_input = input("Enter message to send (or type 'exit' to close the connection): ")
        if user_input.lower() == 'exit':
            ws.close()
            break
        else:
            if user_input == '':
                continue
            ws.send(user_input)

if __name__ == "__main__":
    websocket.enableTrace(False)
    esp32_ipv4_ip_addr = input("Enter WSS addr: ")
    ws = websocket.WebSocketApp('ws://' + esp32_ipv4_ip_addr + ':80/ws',
                                on_open=on_open,
                                on_message=on_message,
                                on_error=on_error,
                                on_close=on_close)

    # Run the WebSocket connection in the main thread
    ws.run_forever()

    print("WebSocket connection closed.")
