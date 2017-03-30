#!/usr/bin/python

import time
import paho.mqtt.publish as mqtt

from sense_hat import SenseHat

sense=SenseHat()

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    client.subscribe("$SYS/#")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))

auth={'username':'phytotron', 'password':'biotronic'}
client_id='sh2mqtt'
qos=2

date=time.strftime('%Y-%m-%d_%H:%M:%S')
print(date)

#Get environnemental data from Sense Hat
temperature=sense.get_temperature()
print("Temperature: %s degC" % temperature)

humidity=sense.get_humidity()
print("Humidity: %s %%rH" % humidity)
temperature_h=sense.get_temperature_from_humidity()
print("Temp. from humidity: %s degC" % temperature_h)

pressure=sense.get_pressure()
print("Pressure: %s mbar" % pressure)
temperature_p=sense.get_temperature_from_pressure()
print("Temp. from pressure: %s degC" % temperature_p)


#Publishes data to MQTT server with retain=true, then disconnect before exiting
messages = [{'topic':"sense_hat/temperature/value",     'payload':temperature,      'qos':qos, 'retain':True},
            {'topic':"sense_hat/temperature/date",      'payload':date,             'qos':qos, 'retain':True},
            {'topic':"sense_hat/humidity/value",        'payload':humidity,         'qos':qos, 'retain':True},
            {'topic':"sense_hat/humidity/date",         'payload':date,             'qos':qos, 'retain':True},
            {'topic':"sense_hat/temperature_h/value",   'payload':temperature_h,    'qos':qos, 'retain':True},
            {'topic':"sense_hat/temperature_h/date",    'payload':date,             'qos':qos, 'retain':True},
            {'topic':"sense_hat/pressure/value",        'payload':pressure,         'qos':qos, 'retain':True},
            {'topic':"sense_hat/pressure/date",         'payload':date,             'qos':qos, 'retain':True},
            {'topic':"sense_hat/temperature_p/value",   'payload':temperature_p,    'qos':qos, 'retain':True},
            {'topic':"sense_hat/temperature_p/date",    'payload':date,             'qos':qos, 'retain':True}]

mqtt.multiple(messages, hostname='localhost', port=1883, client_id=client_id, auth=auth, )
