import joblib
import socket
import asyncio
import sqlite3
import websockets
import process_aps_online

async def server_handler(websocket, path):
    global knn_loc_algorithm

    #connect to database
    db = "devdb.db"
    connection = sqlite3.connect(db)

    while True:
        try:
            request = await websocket.recv()
            print(f"Received Request: {request}")

            process_aps_online.process_request(request)
            fingerprint = process_aps_online.construct_fingerprint_online(connection)
            fingerprint = list(fingerprint)
            #Guard against target being Out Of Range (OOF)
            non_zero = 0
            len_fingerprint = len(fingerprint)
            for j in range(len_fingerprint):
                if fingerprint[j] != 0:
                    non_zero = non_zero + 1
            #############################################

            
            if non_zero < (0.1 * len_fingerprint):
                message = "OOF"
            else:
                message = knn_loc_algorithm.predict([fingerprint])

            await websocket.send(message[0])
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
        start_server = websockets.serve(server_handler, local_ip, 80)
        print(f"WebSocket Server running at ws://{local_ip}:80")
        asyncio.get_event_loop().run_until_complete(start_server)
        asyncio.get_event_loop().run_forever()
    else:
        print("Exiting...")

    
