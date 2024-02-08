import pandas as pd
import sqlite3
import json

detected_aps = None

#convert websocket servers plain text request to df
def process_request(request):
    global detected_aps
    data = []
    scan_results = request.split('\n')
    for result in scan_results:
        components = result.strip().split(',')
        bssid = components[0]
        rssi = int(components[1])
        noccur = int(components[2])
    
        #data.append((bssid, (rssi, noccur)))
        data.append((bssid, rssi))
    
    #dBm_signal = (rssi, noccur)
    detected_aps = pd.DataFrame(data, columns=['bssid', 'dBm_signal'])

    #print(detected_aps)



#contructs and maintains fingeprints ordering according to FV ordering
def construct_fingerprint_online(connection):
    global detected_aps

    
    if detected_aps is None:
        print("detected_aps is not populated")
        return    
    
    
    df = detected_aps[['bssid', 'dBm_signal']]
    fingerprint_elements = df.set_index('bssid')['dBm_signal'].to_dict()


    query = 'SELECT DISTINCT bssid FROM fv_ordering ORDER BY id'
    bssids = pd.read_sql_query(query, connection)['bssid']

    fv_ordering_df = pd.DataFrame({'dBm_signal': [0] * len(bssids)}, index=bssids)
    fv_ordering = fv_ordering_df.to_dict()['dBm_signal']

    for key in  fingerprint_elements.keys():
        if key in fv_ordering.keys():
            fv_ordering[key] = fingerprint_elements[key]

    fingerprint = fv_ordering.values()

    
    #reset wsc request
    detected_aps = None

    return fingerprint
