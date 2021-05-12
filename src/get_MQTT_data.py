import paho.mqtt.client as mqtt
from datetime import datetime
import json
import requests
from requests.exceptions import HTTPError
from pytz import timezone


MQTT_ADDRESS = '192.168.43.162'
MQTT_USER = 'pinkPanther'
MQTT_PASSWORD = 'pinkPanther'
MQTT_TOPIC = 'home/+/+'

TS_API_CHWR_KEY = "XXXXXXXXXXXXXXX"
TS_URL = "https://api.thingspeak.com/"
CHANNEL_ID = "######"

def on_connect(client, userdata, flags, rc):
    """ The callback for when the client receives a CONNACK response from the server."""
    print('Connected with result code ' + str(rc))
    client.subscribe(MQTT_TOPIC)


def on_message(client, userdata, msg):
    """The callback for when a PUBLISH message is received from the server."""
    print(msg.topic + ' ' + str(msg.payload))
    if msg.topic == 'home/data/json':
        body = msg.payload
        print ('Body : ',body)
        dJSON = json.loads(body)
        print ('...JSON: ', dJSON)
        dJSON["created_at"] = datetime.now(timezone('Asia/Kolkata')).isoformat()

        updates = []
        updates.append(dJSON)

        aJSON = updates
        print("aJSON",aJSON)

        headers = {"Content-type": "application/json"}
        print(headers)
        body = json.dumps({"write_api_key":TS_API_CHWR_KEY, "updates": aJSON})
        print (body)
        url = TS_URL + "channels/" + CHANNEL_ID + "/bulk_update.json"
        print(url)
        try:
            r = requests.post(url, data=body, headers=headers, verify=True)
            print(r.status_code)
            r.raise_for_status()
        except HTTPError as http_err:
            print('HTTP error occurred:', http_err)
        except Exception as err:
            print('Other error occurred:', err)
        else:
            print('Success!')

def main():
    mqtt_client = mqtt.Client()
    mqtt_client.username_pw_set(MQTT_USER, MQTT_PASSWORD)
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message

    mqtt_client.connect(MQTT_ADDRESS, 1883)
    mqtt_client.loop_forever()

if __name__ == '__main__':
    print('Into The Main')
    main()
