import joblib
import socket
import asyncio
import websockets
import process_aps_online

async def server_handler(websocket, path):
    global knn_loc_algorithm

    db = "devdb.db"
    connection = sqlite3.connect(db)
    while True:
        try:
            message = await websocket.recv()
            print(f"Received message: {message}")

            process_aps_online.process_wss_res(wss_response)
            fingerprint = process_aps_online.construct_fingerprint(connection)

            #Guard against target being Out Of Range (OOF)
            non_zero = 0
            len_fingeprint = len(fingeprint)
            for j in range(len(len_fingeprint)):
                if fingeprint[j] != 0:
                    non_zero = non_zero + 1
            #############################################

            
            if non_zero < (0.1 * len_fingeprint):
                message = "OOF"
            else:
                message = knn_loc_algorithm.predict(fingerprint)

            await websocket.send(message)
            print(f"Sent message back: {message}")
        except websockets.exceptions.ConnectionClosedOK:
            print("Connection closed by the client.")
            break

if __name__ == "__main__":
    #load model
    knn_loc_algorithm = joblib.load('knn_loc_algorithm.sav')
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
