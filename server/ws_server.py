import socket
import asyncio
import websockets

async def server_handler(websocket, path):
    while True:
        try:
            message = await websocket.recv()
            print(f"Received message: {message}")

            await websocket.send(message)
            print(f"Sent message back: {message}")
        except websockets.exceptions.ConnectionClosedOK:
            print("Connection closed by the client.")
            break

if __name__ == "__main__":
    # Get IP address of machine
    try:
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        s.connect(("8.8.8.8", 80))
        local_ip = s.getsockname()[0]
        s.close()
    except socket.error:
        print("Unable to retrieve local IP address of this machine.")
        local_ip = None

    if local_ip:
        print(f"WebSocket Server running at ws://{local_ip}:8000")

        start_server = websockets.serve(server_handler, local_ip, 8000)

        asyncio.get_event_loop().run_until_complete(start_server)
        asyncio.get_event_loop().run_forever()
    else:
        print("Exiting...")
