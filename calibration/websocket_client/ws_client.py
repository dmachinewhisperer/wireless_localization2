import asyncio
import websockets

async def connect_to_server():
    esp32_ip_address = '172.22.196.117'
    port = 80
    socket_address = f'ws://{esp32_ip_address}:{port}/ws'

    async with websockets.connect(socket_address) as websocket:
        print("WebSocket connection opened")

        # Start a separate task to read user input
        asyncio.ensure_future(send_user_input(websocket))

        # Continuously receive and log messages from the server
        while True:
            response = await websocket.recv()
            print(f"Received from server: {response}")

async def send_user_input(websocket):
    # Continuously read user input and send it to the server
    while True:
        user_input = input("Enter message to send to server (or 'exit' to quit): ")
        if user_input.lower() == 'exit':
            break
        await websocket.send(user_input)

if __name__ == "__main__":
    asyncio.get_event_loop().run_until_complete(connect_to_server())
