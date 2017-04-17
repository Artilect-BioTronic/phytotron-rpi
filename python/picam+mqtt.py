#!/usr/bin/python3

# Prend une photo avec le module Pi Camera et publie le nom du fichier
# sur un broker MQTT

import picamera
import paho.mqtt.publish as mqtt
from time import sleep
import datetime


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
mqtt_qos=2

# Photo parameters
photo_path =            '/home/pi/photos/'
photo_format =          'jpg'
photo_mqtt_topic =      'pi/photo/'

########
# Main #
########

file_date = datetime.datetime.now().replace(microsecond=0).isoformat()
file_name = file_date + "." + photo_format

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
mqtt_data = [{'topic':photo_mqtt_topic+"path",              'payload':photo_path,  'qos':mqtt_qos, 'retain':False},
             {'topic':photo_mqtt_topic+"last_photo_name",   'payload':file_name,   'qos':mqtt_qos, 'retain':False},
             {'topic':photo_mqtt_topic+"last_photo_date",   'payload':file_date,   'qos':mqtt_qos, 'retain':False}]

mqtt.multiple(mqtt_data,
              hostname=  mqtt_server    ,
              port=      mqtt_port      ,
              client_id= mqtt_client_id ,
              auth=      mqtt_auth      )
