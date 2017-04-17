#!/usr/bin/python

# Recupere les donnees du SenseHat et les publie sur un broker MQTT

from sense_hat import SenseHat
import paho.mqtt.publish as mqtt
import datetime

sense=SenseHat()

# MQTT settings
mqtt_server =   'localhost'
mqtt_port=      1883
mqtt_auth=      {'username':'phytotron',
                 'password':'biotronic'}
mqtt_client_id= 'sh2mqtt'
mqtt_qos=2

########
# MAIN #
########

date = datetime.datetime.now().replace(microsecond=0).isoformat()
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


#Get orientation data from IMU
sense.set_imu_config(True, True, True)

orientation_rad=sense.get_orientation_radians()
print("Orientation : Pitch: {pitch}, roll: {roll}, yaw: {yaw}".format(**orientation_rad))
orientation_deg=sense.get_orientation_degrees()
print("Orientation : Pitch: {pitch}, roll: {roll}, yaw: {yaw}".format(**orientation_deg))

compass_north = sense.get_compass()
print("North: %s" % compass_north)
compass_raw = sense.get_compass_raw()
print("Compass (raw) : x: {x}, y: {y}, z: {z}".format(**compass_raw))

gyroscope = sense.get_gyroscope_raw()
print("Gyroscope : x: {x}, y: {y}, z: {z}".format(**gyroscope))

accelerometer = sense.get_accelerometer_raw()
print("Accelerometer : x: {x}, y: {y}, z: {z}".format(**accelerometer))


#Publish data to MQTT server with retain=true, then disconnect before exiting
mqtt_data = [{'topic':"pi/sense_hat/temperature/value",     'payload':temperature,              'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/temperature/date",      'payload':date,                     'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/temperature/unit",      'payload':"degC",                   'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/humidity/value",        'payload':humidity,                 'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/humidity/date",         'payload':date,                     'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/humidity/unit",         'payload':"%RH",                    'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/temperature_h/value",   'payload':temperature_h,            'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/temperature_h/date",    'payload':date,                     'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/temperature_h/unit",    'payload':"degC",                   'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/pressure/value",        'payload':pressure,                 'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/pressure/date",         'payload':date,                     'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/pressure/unit",         'payload':"mbar",                   'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/temperature_p/value",   'payload':temperature_p,            'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/temperature_p/date",    'payload':date,                     'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/temperature_p/unit",    'payload':"degC",                   'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/orientation/rad/pitch", 'payload':orientation_rad['pitch'], 'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/orientation/rad/roll",  'payload':orientation_rad['roll'],  'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/orientation/rad/yaw",   'payload':orientation_rad['yaw'],   'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/orientation/rad/date",  'payload':date,                     'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/orientation/rad/unit",  'payload':"rad",                    'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/orientation/deg/pitch", 'payload':orientation_deg['pitch'], 'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/orientation/deg/roll",  'payload':orientation_deg['roll'],  'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/orientation/deg/yaw",   'payload':orientation_deg['yaw'],   'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/orientation/deg/date",  'payload':date,                     'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/orientation/deg/unit",  'payload':"degree",                 'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/compass/x",             'payload':compass_raw['x'],         'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/compass/y",             'payload':compass_raw['y'],         'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/compass/z",             'payload':compass_raw['z'],         'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/compass/north",         'payload':compass_north,            'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/compass/date",          'payload':date,                     'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/compass/unit",          'payload':"micro Tesla",            'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/gyroscope/x",           'payload':gyroscope['x'],           'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/gyroscope/y",           'payload':gyroscope['y'],           'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/gyroscope/z",           'payload':gyroscope['z'],           'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/gyroscope/date",        'payload':date,                     'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/gyroscope/unit",        'payload':"rad/s",                  'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/accelerometer/x",       'payload':accelerometer['x'],       'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/accelerometer/y",       'payload':accelerometer['y'],       'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/accelerometer/z",       'payload':accelerometer['z'],       'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/accelerometer/date",    'payload':date,                     'qos':mqtt_qos, 'retain':True},
             {'topic':"pi/sense_hat/accelerometer/unit",    'payload':"rad/s",                  'qos':mqtt_qos, 'retain':True}]

mqtt.multiple(mqtt_data,
              hostname=  mqtt_server    ,
              port=      mqtt_port      ,
              client_id= mqtt_client_id ,
              auth=      mqtt_auth      )

