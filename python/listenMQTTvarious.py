#!/usr/bin/python

from __future__ import print_function
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import serial
import time
from datetime import datetime
import ast
import sys, os, getopt
import subprocess

# IP address of MQTT broker
# example of free MQTT broker:  'iot.eclipse.org'
hostMQTT='localhost'
portMQTT=1883
filePassMqtt="./passMqtt.txt"

clientId='myNameOfClient'

logfile=sys.stdout
logStartTime=0.

devSerial='/dev/ttyUSB0'   # serial port the arduino is connected to
baudRate=9600

mqTopic1='phytotron/camera/oh/newPicture'
mqTopic2='phytotron/shellCmd/oh/wifiID'
mqTopic3='phytotron/admin/oh/askTime'
#cmdTopic1="/home/arnaud/Workspaces/Arduino/PythonScripts/picam+mqttFake.py"
cmdTopic1="/home/pi/python/picam+mqtt.py"
cmdTopic2="iwgetid -r"
cmdTopic2b="ifconfig wlan0 | sed -n -e 's/.*inet adr://' -e 's/ *Bcast.*//p'"
mqRepShift2=['oh', 'pysys']
mqRepTopic3='phytotron/admin/py/piClock'


# serial msg to arduino begin  with  prefTopic2 / prefTopic1 and end with endOfLine
prefTopic1='CM+'
prefTopic2='AT+'
endOfLine='\n'

sleepBetweenLoop=1    # sleep time (eg: 1s) to slow down loop
sleepResponse=0.09    # sleep to leave enough time for the arduino to respond immediately

namePy='py0'
topFromPy= namePy + '/'


# use to sort log messages
def logp (msg, gravity='trace'):
	print('['+gravity+']' + msg, file=logfile)

# log file must not grow big
# I need to overwrite it often
def reOpenLogfile(logfileName):
	"re open logfile, I do it because it must not grow big"
	global logStartTime, logfile
	#
	if logfileName != '' :
		try:
			# I close file if needed
			if ( not logfile.closed) and (logfile.name != '<stdout>') :
				logfile.close()
			# file will be overwritten
			if (logfileName != '<stdout>') :
				logfile = open(logfileName, "w", 1)
			logStartTime = time.time()
			logp('logStartTime:' + time.asctime(time.localtime(time.time())), 'info')
		except IOError:
			print('[error] could not open logfile:' + logfileName)
			sys.exit(3)
	else :
		print('I cant re open logfile. name is empty')


def read_args(argv):
    # optional args have default values above
    global logfile, hostMQTT, baudRate, namePy, mqTopic1, mqTopic2, devSerial
    logfileName = ''
    try:
        opts, args = getopt.getopt(argv,"hl:b:r:n:t:u:d:",["logfile=","broker=","baudrate=","namepy=","mqTopic1=","mqTopic2=","devSerial="])
    except getopt.GetoptError:
        print ('serial2MQTTduplex.py -l <logfile> -n <namepy> -b <broker> -r <baudrate> -t <mqTopic1> -u <mqTopic2> -d <devSerial>')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print ('serial2MQTTduplex.py -l <logfile> -n <namepy> -b <broker> -r <baudrate> -t <mqTopic1> -u <mqTopic2> -d <devSerial>')
            sys.exit()
        elif opt in ("-l", "--logfile"):
            logfileName = arg
        elif opt in ("-r", "--baudrate"):
            baudRate = arg
        elif opt in ("-b", "--broker"):
            hostMQTT = arg
        elif opt in ("-n", "--namepy"):
            namePy = arg
        elif opt in ("-t", "--mqTopic1"):
            mqTopic1 = arg
        elif opt in ("-u", "--mqTopic2"):
            mqTopic2 = arg
        elif opt in ("-d", "--devSerial"):
            devSerial = arg
    logp('logfile is '+ logfileName, 'debug')
    logp('broker is '+ hostMQTT, 'debug')
    logp('baudrate is '+ str(baudRate), 'debug')
    logp('namepy is '+ namePy, 'debug')
    logp('mqTopic1 is '+ mqTopic1, 'debug')
    logp('mqTopic2 is '+ mqTopic2, 'debug')
    logp('devSerial is '+ devSerial, 'debug')
    # I try to open logfile
    if logfileName != '' :
        reOpenLogfile(logfileName)


logStartTime = time.time()
if __name__ == "__main__":
	read_args(sys.argv[1:])


# if logfile is old, we remove it and overwrite it
#   because it must not grow big !
def checkLogfileSize(logfile):
    "if logfile is too big, we remove it and overwrite it because it must not grow big !"
    if (logfile.name != '<stdout>') and (os.path.getsize(logfile.name) > 900900):
        reOpenLogfile(logfile.name)

def readListSketchFromFile(fileName):
    "read filename and append valid dict lines in returned dict"
    logp('reading list of arguments for sketch in file : ' + fileName)
    try:
        fileListSketch = open(fileName, 'r')
    except:
        logp('could not open list of sketch configuration file: ' + fileName, 'error')
        return {}
    #
    strLines = fileListSketch.readlines()
    fileListSketch.close()
    dSketch = {"username":"user", "password":"pass"}
    for strl in strLines:
        try:
            if (strl[0] != '#'):
                itemDict = ast.literal_eval(strl)
                if (type(itemDict) == type({})):
                    dSketch.update(itemDict)
                else:
                    logp ('line NOT dict:' + strl)
        except:
          logp('line fails as dict:' + strl)
    return dSketch


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, rc):
    print("Connected to mosquitto with result code "+str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    mqttc.subscribe([(mqTopic1, 0) , (mqTopic2+'/#', 0), (mqTopic3, 0)])
    mqttc.message_callback_add(mqTopic1, on_message_mqTopicOH1)
    mqttc.message_callback_add(mqTopic2+'/#', on_message_mqTopicOH2)
    mqttc.message_callback_add(mqTopic3, on_message_mqTopicOH3)


# The callback for when a PUBLISH message is received from the server.
# usually we dont go to  on_message
#  because we go to a specific callback that we have defined for each topic
def on_message(client, userdata, msg):
    logp('msg:' +msg.topic+" : "+str(msg.payload), 'unknown msg')


# The callback for when the server receives a message of  mqTopic1.
def on_message_mqTopicOH1(client, userdata, msg):
    logp("mqTopicOH:"+msg.topic+" : "+str(msg.payload), 'info')
    try :
        subprocess.call([cmdTopic1, str(msg.payload)])
    except:
      logp('exception managing msg:'+msg.topic+" : "+str(msg.payload), 'com error')

# The callback for when the server receives a message of  mqTopic2.
def on_message_mqTopicOH2(client, userdata, msg):
    logp("mqTopicOH:"+msg.topic+" : "+str(msg.payload), 'info')
    output2=''; output2b='';
    try :
        output2 = subprocess.Popen(cmdTopic2, shell=True, stdout=subprocess.PIPE).stdout.read()
        output2b = subprocess.Popen(cmdTopic2b, shell=True, stdout=subprocess.PIPE).stdout.read()
    except:
        logp('exception managing msg:'+msg.topic+" : "+str(msg.payload), 'com error')
        retTopic = mqTopic2.replace(mqRepShift2[0], mqRepShift2[1]) + '/KO'
        mqttc.publish(retTopic, output2)
    retTopic = mqTopic2.replace(mqRepShift2[0], mqRepShift2[1])
    mqttc.publish(retTopic + '/essid', output2)
    mqttc.publish(retTopic + '/IP', output2b)
    

# The callback for when the server receives a message of  mqTopic3.
def on_message_mqTopicOH3(client, userdata, msg):
    logp("mqTopicOH:"+msg.topic+" : "+str(msg.payload), 'info')
    # we publish system time  (module datetime) au format 2015-05-21T12:34:56 (16 char)
    mqttc.publish(mqRepTopic3, datetime.now().isoformat()[:19])


# read all available messages from arduino
def readArduinoAvailableMsg(seri):
    while seri.inWaiting():
        # because of readline function, dont forget to open with timeout
        response = seri.readline().replace('\n', '')
        #logp ("answer is:" + response, 'debug')
        if response.startswith(prefTopic1):
            # prefTopic1: message to send to mqtt
            # I dont analyse those messages, I transmit to mqtt
            tags = response.replace(prefTopic1, '',1).split(':')
            # security: I strip wildcard in topic before publishing
            topic = tags[0].replace('+', '').replace('#', '')
            pyTopic = mqTopic1 + topFromPy + topic
            if len(tags) > 1 :
                value = ':'.join(tags[1:])
            else :
                value = ''
            # trace
            logp('{} = {}'.format(pyTopic, value), 'to MQTT')
            mqttc.publish(pyTopic, value, retain=True)
        elif response.startswith(prefTopic2):
            # prefTopic2: message to send to mqtt
            # I dont analyse those messages, I transmit to mqtt
            tags = response.replace(prefTopic2, '',1).split(':')
            # security: I strip wildcard in topic before publishing
            topic = tags[0].replace('+', '').replace('#', '')
            pyTopic = mqTopic2 + topFromPy + topic
            if len(tags) > 1 :
                value = ':'.join(tags[1:])
            else :
                value = ''
            # trace
            logp('{} = {}'.format(pyTopic, value), 'to MQTT')
            mqttc.publish(pyTopic, value, retain=True)
        else :
            # I dont analyse, but I print
            logp (response, 'unknown from '+devSerial)



#---------------------------------------------------
#                   mosquitto
#---------------------------------------------------

# read password from file  passMqtt.txt
authInFile = readListSketchFromFile(filePassMqtt)

# connection to mosquitto
mqttc = mqtt.Client("", True, None, mqtt.MQTTv31)
mqttc.on_message = on_message
mqttc.on_connect = on_connect
if (authInFile.has_key('username') and authInFile.has_key('password')) :
    mqttc.username_pw_set(authInFile["username"], authInFile["password"])

cr = mqttc.connect(hostMQTT, port=portMQTT, keepalive=60, bind_address="")
mqttc.loop_start()


# infinite loop
while True:
	time.sleep(sleepBetweenLoop)
	#check size
	checkLogfileSize(logfile)



