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

logfile=sys.stdout
logStartTime=0.

devSerial='/dev/ttyUSB0'   # serial port the arduino is connected to
baudRate=9600

mqTopic1='/phytotron/neopixel/'
mqTopic2='/phytotron/neopixelUnused/'

# serial msg to arduino begin  with  prefTopic2 / prefTopic1 and end with endOfLine
prefTopic1='CM+'
prefTopic2='AT+'
endOfLine='\n'
# arduino responds with those 2 kind of messages
msg2py='2py'
msg2mqtt='2mq'

sleepBetweenLoop=1    # sleep time (eg: 1s) to slow down loop
sleepResponse=0.09    # sleep to leave enough time for the arduino to respond immediately

namePy='py0'
topFromPy= namePy + '/'
topFromOH='oh/'
topFromSys='sys/'


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
    logp('devserial is '+ devSerial, 'debug')
    # I try to open logfile
    if logfileName != '' :
        reOpenLogfile(logfileName)


logStartTime = time.time()
if __name__ == "__main__":
	read_args(sys.argv[1:])


# if logfile is old, we remove it and overwrite it
#   because it must not grow big !
def checkLogfileSize(logfile):
	"if logfile is old, we remove it and overwrite it because it must not grow big !"
	global logStartTime
	if (time.time() - logStartTime) > 600:
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
    dSketch = {"username":"phytotron", "password":"biotronic"}
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
    mqttc.subscribe(mqTopic1+ topFromOH +'#')
    mqttc.message_callback_add(mqTopic1+ topFromOH+ '#', on_message_mqTopicOH)
    mqttc.subscribe(mqTopic2+ topFromOH +'#')
    mqttc.message_callback_add(mqTopic2+ topFromOH+ '#', on_message_mqTopicOH)
    mqttc.subscribe(mqTopic1+ topFromSys +'#')
    mqttc.message_callback_add(mqTopic1+ topFromSys+ '#', on_message_mqTopicSys)

# The callback for when a PUBLISH message is received from the server.
# usually we dont go to  on_message
#  because we go to a specific callback that we have defined for each topic
def on_message(client, userdata, msg):
    logp('msg:' +msg.topic+" : "+str(msg.payload), 'info gen')
    cmd2arduino = prefTopic2 + str(msg.payload) + endOfLine
    ser.write(cmd2arduino)
    # I immediately wait a moment for the arduino to respond
    time.sleep(sleepResponse)
    readArduinoAvailableMsg(ser)

# The callback for when a PUBLISH message is received from the server.
# usually  we go to a specific callback that we have defined for each topic
def on_message_mqTopicOH(client, userdata, msg):
    logp("mqTopicOH:"+msg.topic+" : "+str(msg.payload), 'info')
    try :
        if ( len(msg.payload) == 0) :
            logp("topic with empty payload:"+msg.topic, 'info')
            return
        if msg.topic.startswith(mqTopic1+topFromOH+ 'mode') :
            listNumCmd = ['0', '1', 'b', 'r', 'g', 'w']
            if ( msg.payload.isdigit() and (0<= int(msg.payload)) and (int(msg.payload) <=5)) :
                theCmd = listNumCmd[int(msg.payload)]
            else :
                theCmd = str(int(msg.payload))
            ser.write(theCmd)
        elif msg.topic.startswith(mqTopic1+topFromOH+ 'intensity') :
            if (not msg.payload.isdigit()) :
                return
            numIntensity = int(msg.payload)
            # if it cannot be converted to char
            if (numIntensity < 0 or 255 < numIntensity):
                return
            ser.write(chr(numIntensity))
    except:
      logp('exception parsing MQTT msg:'+msg.topic+" : "+str(msg.payload), 'com error')
    time.sleep(sleepResponse)
    readArduinoAvailableMsg(ser)

# The callback for when a PUBLISH message is received from the server.
# usually  we go to a specific callback that we have defined for each topic
def on_message_mqTopicSys(client, userdata, msg):
    logp("spec callbackSys,"+msg.topic+":"+str(msg.payload), 'info topicSys')
    cmd = msg.topic.replace(mqTopic1, '').replace(topFromSys, '').replace('#','')
    cmd2arduino = prefTopic2 + cmd + ':' + str(msg.payload)+ endOfLine
    ser.write(cmd2arduino)
    # I immediately wait a moment for the arduino to respond
    time.sleep(sleepResponse)
    readArduinoAvailableMsg(ser)

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
        elif response.startswith(msg2py):
            # msg2py: message 2 python only
            # python use this to check connection with arduino
            logp ('msg2py '+response.replace(msg2py, '',1), 'info')
        elif response.startswith(msg2mqtt):
            # msg2mqtt: message to send to mqtt
            # I dont analyse those messages, I transmit to mqtt
            tags = response.replace(msg2mqtt, '',1).split(':')
            topic = tags[0].replace('+', '').replace('#', '')
            pyTopic = mqTopic1 + topFromPy + topic
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


# connection to arduino
# I use 9600, because I had many pb with pyserial at 38400 !!!
ser = serial.Serial(devSerial, baudrate=baudRate, timeout=0.2)
logp (str(ser), 'info')
# when we open serial to an arduino, this reset the board; it needs ~3s
time.sleep(0.2)
#I empty arduino serial buffer
response = ser.readline()
logp ("arduino buffer garbage: " + str(response), 'info')
time.sleep(3)

# loop to get connection to arduino


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

#mqttc.connect('iot.eclipse.org', port=1883, keepalive=60, bind_address="")
cr = mqttc.connect(hostMQTT, port=portMQTT, keepalive=60, bind_address="")
mqttc.loop_start()


# infinite loop
# I regulary query a new measure value from arduino
# then I publish it
while True:
	time.sleep(sleepBetweenLoop)
	#queryValue()  # currently the querry is made by OpenHab
	readArduinoAvailableMsg(ser)
	#check size
	checkLogfileSize(logfile)



