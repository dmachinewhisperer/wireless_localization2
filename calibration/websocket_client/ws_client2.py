import websocket
import _thread
import time

def on_message(ws, message):
    print(f"Received message: {message}")

def on_error(ws, error):
    print(f"Error occurred: {error}")

def on_close(ws, close_status_code, close_msg):
    print(f"Connection closed with status code {close_status_code}: {close_msg}")

def on_open(ws):
    print("Opened connection")

    # Start a new thread for user input
    _thread.start_new_thread(send_user_input, (ws,))

def send_user_input(ws):
    while True:
        user_input = input("Enter message to send (or type 'exit' to close the connection): ")
        if user_input.lower() == 'exit':
            ws.close()
            break
        else:
            ws.send(user_input)

if __name__ == "__main__":
    websocket.enableTrace(False)
    ws = websocket.WebSocketApp('ws://172.22.194.205:80/ws',
                                on_open=on_open,
                                on_message=on_message,
                                on_error=on_error,
                                on_close=on_close)

    # Run the WebSocket connection in the main thread
    ws.run_forever()

    print("WebSocket connection closed.")
