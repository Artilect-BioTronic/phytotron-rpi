#!/usr/bin/python3

# Récupère des données formatées sur un port série et les publie sur
# un broker MQTT

import serial
import paho.mqtt.client as mqtt
import json

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("$SYS/#")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

###

ser = serial.Serial('/dev/ttyACM0', 115200, timeout=2)

mqttc = mqtt.Client()

mqttc.username_pw_set('phytotron', password='biotronic')
mqttc.on_connect = on_connect
mqttc.on_message = on_message
mqttc.connect('localhost', port=1883, keepalive=60)
mqttc.loop_start()

while True :
    line = ser.readline().decode("utf-8").replace('\r\n', '')
    
    if len(line) > 0 and line[0] == "{" and line[len(line)-1] == "}":
        d = json.loads(line.replace("'", "\""))
        print("topic   : ", d['topic'])
        print("payload : ", d['payload'])
        print("QoS     : ", int(d['qos']))
        print("Retain  : ", bool(d['retain']))
                
        mqttc.publish(d['topic'], payload=d['payload'], qos=int(d['qos']), retain=bool(d['retain']))

