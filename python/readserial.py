#!/usr/bin/python3

# Récupère des données formatées sur un port série et les publie sur
# un broker MQTT

import serial
import json

ser = serial.Serial('/dev/ttyACM0', 115200, timeout=2)

while True :
    line = ser.readline()
    print(line)
