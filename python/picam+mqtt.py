#!/usr/bin/python3

# Prend une photo avec le module Pi Camera et publie le nom du fichier
# sur un broker MQTT

import picamera
import paho.mqtt.publish as publish
from datetime import datetime
from time import sleep


# Camera settings
camera_resolution=(2592, 1944)
camera_warmup_time=2            #in seconds
#camera_iso=800

# MQTT settings
mqtt_server =   'localhost'
mqtt_port=      1883
mqtt_auth=      {'username':'phytotron',
                 'password':'biotronic'}
mqtt_client_id= 'picamera'


# Photo parameters
photo_path =            '/home/pi/photos/'
photo_name_format =     '%Y-%m-%d_%H-%M-%S.jpg'     #strftime() format
photo_mqtt_topic = 'pi/photo/'

########
# Main #
########

file_name = datetime.now().strftime(photo_name_format)

# Taking a photo
#################
cam = picamera.PiCamera(resolution = camera_resolution)

# Warm-up
cam.start_preview()
sleep(camera_warmup_time)

print(photo_path+file_name)

cam.capture(photo_path+file_name)

# MQTT publication
###################
mqtt_data = [{'topic':photo_mqtt_topic+"path",         'payload':photo_path,  'qos':2, 'retain':False},
             {'topic':photo_mqtt_topic+"last_photo",   'payload':file_name,   'qos':2, 'retain':False}]

publish.multiple(mqtt_data,
                 hostname=  mqtt_server,
                 port=      mqtt_port,
                 client_id= mqtt_client_id,
                 auth=      mqtt_auth)
