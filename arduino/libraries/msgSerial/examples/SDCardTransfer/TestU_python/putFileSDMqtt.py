#!/usr/bin/python

from __future__ import print_function
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import serial
import time
import ast
import sys, getopt


# IP address of MQTT broker
# example of free MQTT broker:  'iot.eclipse.org'
hostMQTT='localhost'
portMQTT=1883
filePassMqtt="./passMqtt.txt"

clientId='myNameOfClient'

#devSerial='/dev/ttyACM0'   # serial port the arduino is connected to
fileSDN = "COM.CSV"
fileHDN = ""
fileHD = sys.stdout
moveTo = ''
moveTo2 = ''

cmdSendValue='SendValue'

mqTopicSend='phytotron/SDlog/send/'
mqTopicRec ='phytotron/SDlog/receive/'



# use to sort log messages
def logp (msg, gravity='trace'):
	print('['+gravity+']' + msg)


def read_args(argv):
    # optional args have default values above
    global fileSDN, fileHDN, moveTo, moveTo2, hostMQTT, portMQTT
    try:
        opts, args = getopt.getopt(argv,"hf:g:m:n:b:p:",["fileInSDCard=","fileOnHD=","moveTo=","moveTo2=","broker=","portBroker="])
    except getopt.GetoptError:
        print ('putFileSDMqtt.py  -f <fileInSDCard> -g <fileOnHD> -m <moveTo> -n <moveTo2> -b <broker> -p <portBroker>')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print ('putFileSDMqtt.py  -f <fileInSDCard> -g <fileOnHD> -m <moveTo> -n <moveTo2> -b <broker> -p <portBroker>')
            sys.exit()
        elif opt in ("-b", "--broker"):
            hostMQTT = arg
        elif opt in ("-p", "--portBroker"):
            portMQTT = arg
        elif opt in ("-f", "--fileInSDCard"):
            fileSDN = arg
        elif opt in ("-g", "--fileOnHD"):
            fileHDN = arg
        elif opt in ("-m", "--moveTo"):
            moveTo = arg
        elif opt in ("-n", "--moveTo2"):
            moveTo2 = arg
    logp('fileInSDCard is '+ fileSDN, 'debug')
    logp('fileOnHD is ' + fileHDN, 'debug')
    logp('moveTo is '   + moveTo, 'debug')
    logp('moveTo2 is '  + moveTo2, 'debug')
    logp('broker is '+ str(hostMQTT), 'debug')
    logp('port of broker is '+ str(portMQTT), 'debug')


if __name__ == "__main__":
	read_args(sys.argv[1:])


# open file on Hard Disk
def openHDFile(afileHDName, access_mode='r+'):
	if (afileHDName != '') and (afileHDName != '<stdout>') :
		try:
			# I close file if needed
			#if ( not fileHD.closed ) and (fileHD.name != '<stdout>') :
			#    fileHD.close()
			# file will be overwritten
			if (afileHDName != '<stdout>') :
				fileOpened = open(afileHDName, access_mode, 1)
			logp('open fileHD ' + time.asctime(time.localtime(time.time())), 'info')
			return fileOpened
		except IOError:
			print('[error] could not open fileHD:' + afileHDName)
			sys.exit(3)
	else :
		print('I cant open destination file. name is empty')
	return sys.stdout


def readDictFromFile(fileName):
    "read filename and append valid dict lines in returned dict"
    logp('reading dictionary in file : ' + fileName)
    try:
        fileListSketch = open(fileName, 'r')
    except:
        logp('could not open dictionary file: ' + fileName, 'error')
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
    mqttch.subscribe([ (mqttch.topicRec+'#', 0) ])
    mqttch.message_callback_add(mqttch.topicRec+'#', on_message_topicRec)


# The callback for when a PUBLISH message is received from the server.
# usually we dont go to  on_message
#  because we go to a specific callback that we have defined for each topic
def on_message(client, userdata, msg):
    logp('msg:' +msg.topic+" : "+str(msg.payload), 'unknown msg')


# The callback for when the server receives a message of  topicRec.
def on_message_topicRec(client, userdata, msg):
    """we cut off  generic part of topic, and we store the remaining
       ex: phytotron/SDlog/receive/read/OK ->
           read/OK as key   and msg.payload as value """
    key = msg.topic[len(mqttch.topicRec):]
    # I dont want to store numerous keys  (sign of bug)
    if len(mqttch.store) <= mqttch.maxCmdStore :
        mqttch.store[key] = msg.payload
        #logp('I ve stored response:'+str(mqttch.store), 'trace')
    else :
        logp('too many topics of msg. msg:' +msg.topic+" : "+str(msg.payload), 'error')


class MQTTChannel(mqtt.Client):
    """L objet va permettre la communication avec MQTT
    Tous les messages, aller et retour, utiliserons la meme racine de topic"""
    
    maxCmdStore = 10   # I wont memorize too many different command in store
    
    def __init__(self, aTopicSend, aTopicRec, 
                 client_id="", clean_session=True, userdata=None, protocol=mqtt.MQTTv31):
        mqtt.Client.__init__(self, client_id=client_id, clean_session=clean_session,
                            userdata=userdata, protocol=protocol)
        self.topicSend = aTopicSend
        self.topicRec = aTopicRec
        self.store = {}
        self.timeSleepResponse = 0.02
    
    def pubGetMqtt(self, aCmd, aLoad, aRetain=False):
        """I send msg, then I wait for response.
        The response will be detected by subscribe call back and 
        stored in store dict."""
        #
        # I empty former answer
        if self.store.has_key(aCmd+'/OK'):
            del self.store[aCmd+'/OK']
        if self.store.has_key(aCmd+'/KO'):
            del self.store[aCmd+'/KO']
        #
        # I send cmd
        self.publish(self.topicSend + aCmd, aLoad, retain=aRetain)
        #
        # I have to get back response
        # I wait, I give some time to transmit msg
        time.sleep(self.timeSleepResponse)
        timeStartWait = time.time()
        # when a response is received, the OK key (or KO key) will be present
        while ( (not self.store.has_key(aCmd+'/OK')) and 
                (not self.store.has_key(aCmd+'/KO')) and 
                (time.time() - timeStartWait) < 1. ) :
            # I put a small sleep to release CPU
            time.sleep(self.timeSleepResponse / 10.)
        #
        if (self.store.has_key(aCmd+'/OK')):
            return [self.store[aCmd+'/OK'], 0]
        if (self.store.has_key(aCmd+'/KO')):
            return [self.store[aCmd+'/KO'], -1]
        else:    #timeout
            return ['', -2]
    
    


#---------------------------------------------------
#                   mosquitto
#---------------------------------------------------

# read password from file  passMqtt.txt
authInFile = readDictFromFile(filePassMqtt)

# connection to mosquitto
mqttch = MQTTChannel(mqTopicSend, mqTopicRec, "", True, None, mqtt.MQTTv31)
mqttch.on_message = on_message
mqttch.on_connect = on_connect
if (authInFile.has_key('username') and authInFile.has_key('password')) :
    mqttch.username_pw_set(authInFile["username"], authInFile["password"])

cr = mqttch.connect(hostMQTT, port=portMQTT, keepalive=60, bind_address="")
mqttch.loop_start()
# I sleep to empty retained messages of mqtt
time.sleep(mqttch.timeSleepResponse *5)


# open modes are different from basic SD.h library
# open modes are (0X08 to 0X01): O_SYNC | O_APPEND | O_WRITE | O_READ
# open modes are (0X80 to 0X10): O_EXCL | O_CREAT | O_AT_END | O_TRUNC

# if there is no move, we erase former file
if (moveTo == '') and (moveTo2 == '') :
    # open r/w create and trunc
    modeOpenSD = "rwct" # 1+2+16+64
else :
    # open r/w create
    modeOpenSD = "rwc" # 1+2+64

# if you want to append, use option --move END

[response, cr] = mqttch.pubGetMqtt("open", fileSDN + "," + str(modeOpenSD))
if cr < 0 :
   print ('open : ', fileSDN, ' failed', '\nbye')
   sys.exit(-1)

print ('open : ', fileSDN , ':', cr)

# We move to a special position in file
# we start by moving to begin of file (open as write provoke append at end)
if (moveTo != '') or (moveTo2 != '') :
   [response, cr] = mqttch.pubGetMqtt("move", "BEGIN")

if moveTo != '':
   [response, cr] = mqttch.pubGetMqtt("move", moveTo)

if moveTo2 != '':
   [response, cr] = mqttch.pubGetMqtt("move", moveTo2)


fileHD = openHDFile(fileHDN, 'r')
print ('file: '+ fileHDN + ' opened')
strLines = fileHD.readlines()
fileHD.close()
for strl in strLines:
    [response, cr] = mqttch.pubGetMqtt("writeln", strl.replace('\n',''))
    #logp('writing:' + strl, 'info')
    if cr != 0:
        print('response and cr !=0:%s, %d' % (response,cr))
        break


[response, cr] = mqttch.pubGetMqtt("close", "")
print ('close, cr: ', cr)


#sys.exit(0)

#===============================

