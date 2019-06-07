#!/usr/bin/python

from __future__ import print_function
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import serial
import time
import ast, re
import os, sys, getopt

# IP address of MQTT broker
# example of free MQTT broker:  'iot.eclipse.org'
hostMQTT='localhost'
portMQTT=1883
filePassMqtt="./passMqtt.txt"
clientId='myNameOfClient'

logfile=sys.stdout

devSerial='/dev/ttyUSB0'   # serial port the arduino is connected to
# I often use 9600, because I had many pb with pyserial at 38400 !!!
baudRate=9600

# we convert  mqtt topic  to prefix for arduino
prefFromTopic = {'phytotron/oh/':'SD+', 'phytotron/sys/':'AT+'}
# arduino responds with those prefixes, we convert in topics
topicFromPref = {'SD+':'phytotron/py/', 'AT+':'phytotron/py/'}

# serial msg to arduino begin  with  prefix  and end with endOfLine
# for instance:  AT+ls\n
endOfLine='\n'

sleepBetweenLoop=1    # sleep time (eg: 1s) to slow down loop
sleepResponse=0.1    # sleep to leave enough time for the arduino to respond immediately


# use to sort log messages
def logp (msg, gravity='trace'):
	print('['+gravity+']' + msg, file=logfile)

# log file must not grow big
# I need to overwrite it often
def reOpenLogfile(logfileName):
    "re open logfile, I do it because it must not grow big"
    global logfile
    #
    if logfileName != '' :
        try:
            # I close file if needed
            if ( not logfile.closed) and (logfile.name != '<stdout>') :
                logfile.close()
            # file will be overwritten
            if (logfileName != '<stdout>') :
                logfile = open(logfileName, "a", 1)
                logp('logStartTime:' + time.asctime(time.localtime(time.time())), 'info')
        except IOError:
            print('[error] could not open logfile:' + logfileName)
            sys.exit(3)
    else :
        print('I cant re open logfile. name is empty')


def read_args(argv):
    # optional args have default values above
    global logfile, hostMQTT, portMQTT, baudRate, prefFromTopic, topicFromPref, devSerial
    logfileName = ''
    try:
        opts, args = getopt.getopt(argv,"hl:b:p:d:r:t:u:",["logfile=","broker=","portBroker=","devSerial=","baudrate=","topicFromPref=","prefFromTopic="])
    except getopt.GetoptError:
        print ('serial2MQTTduplex.py -l <logfile> -b <broker> -p <portBroker> -d <devSerial> -r <baudrate> -t <topicFromPref> -u <prefFromTopic>')
        sys.exit(2)
    for opt, arg in opts:
        if opt == '-h':
            print ('serial2MQTTduplex.py -l <logfile> -b <broker> -p <portBroker> -d <devSerial> -r <baudrate> -t <topicFromPref> -u <prefFromTopic>')
            sys.exit()
        elif opt in ("-l", "--logfile"):
            logfileName = arg
        elif opt in ("-d", "--devSerial"):
            devSerial = arg
        elif opt in ("-r", "--baudrate"):
            baudRate = arg
        elif opt in ("-b", "--broker"):
            hostMQTT = arg
        elif opt in ("-p", "--portBroker"):
            portMQTT = arg
        elif opt in ("-u", "--prefFromTopic"):
            prefFromTopic = ast.literal_eval(arg)
        elif opt in ("-t", "--topicFromPref"):
            topicFromPref = ast.literal_eval(arg)
    logp('logfile is '+ logfileName, 'debug')
    logp('broker is '+ str(hostMQTT), 'debug')
    logp('port of broker is '+ str(portMQTT), 'debug')
    logp('baudrate is '+ str(baudRate), 'debug')
    logp('prefFromTopic is '+ str(prefFromTopic), 'debug')
    logp('topicFromPref is '+ str(topicFromPref), 'debug')
    logp('devserial is '+ devSerial, 'debug')
    # I try to open logfile
    if logfileName != '' :
        reOpenLogfile(logfileName)


if __name__ == "__main__":
	read_args(sys.argv[1:])


# if logfile is too big, we remove it and overwrite it
#   because it must not grow big !
def checkLogfileSize(logfile):
    "if logfile is too big, we remove it and overwrite it because it must not grow big !"
    if (logfile.name != '<stdout>') and (os.path.getsize(logfile.name) > 900900):
        reOpenLogfile(logfile.name)


# returns empty dict if file is useless
def readDictPassFromFile(fileName):
    "read filename and append valid dict lines in returned dict"
    return updateDictFromFile(fileName)

# update  dictionnary inDict with  the dictionnary  read in fileName
def updateDictFromFile(fileName, inDict={}):
	"read filename and append valid dict lines in returned dict"
	try:
		fileDict = open(fileName, 'r')
	except:
		logp('could not open dictionnary file: ' + fileName, 'error')
		return inDict
	#
	try:
	    newDict = ast.literal_eval(fileDict.read())
	except:
		logp('could not eval dictionnary in file: ' + fileName, 'error')
		return inDict
	#
	# it must be dictionnary
	if (type(newDict) != type({})):
		logp('It was not of type dictionnary in file: ' + fileName, 'error')
		return inDict
	#
	inDict.update(newDict)
	#
	fileDict.close()
	return inDict

# check the correspondance  pref <---> topic   are correct
# check topics end with /
# topic keys in prefFromTopic must not be in items of topicFromPref
def checkTopicAndPref(aPrefFromTopic, aTopicFromPref):
    for itopic in aPrefFromTopic.keys():
        if type(itopic) != type(''):
            logp("topic:"+itopic+" must be a string", 'error')
            return -1
        if not itopic.endswith('/'):
            logp("topic:"+itopic+" must end with /", 'error')
            return -2
        if aTopicFromPref.has_key(itopic):
            logp("topic:"+itopic+" can not be in both dict; it would create loop", 'error')
            return -3
    return 0


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected to mosquitto with result code "+str(rc))
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    for itopic in prefFromTopic.keys():
        if type(itopic) != type(''):
            continue
        mqttc.subscribe(itopic +'#')
        logp('subscribing topic:'+itopic +'#', 'trace')
        mqttc.message_callback_add(itopic + '#', on_message_mqTopic)

# The callback for when a PUBLISH message is received from the server.
# usually we dont go to  on_message
#  because we go to a specific callback that we have defined for each topic
def on_message(client, userdata, msg):
    logp('msg:' +msg.topic+" : "+str(msg.payload), 'info')
    cmd2arduino = prefTopic2 + str(msg.payload) + endOfLine
    ser.write(cmd2arduino.encode('ascii','ignore'))
    # I immediately wait a moment for the arduino to respond
    time.sleep(sleepResponse)
    readArduinoAvailableMsg(ser)

# The callback for when a PUBLISH message is received from the server.
# usually  we go to a specific callback that we have defined for each topic
def on_message_mqTopic(client, userdata, msg):
    logp("mqTopic:"+msg.topic+" : "+str(msg.payload), 'info')
    mqTopic = 'notFound'
    prefTopic = 'noPref'
    for itopic in prefFromTopic.keys():
        if msg.topic.startswith(itopic) :
            mqTopic   = itopic
            prefTopic = prefFromTopic[itopic]
    cmd = msg.topic.replace(mqTopic, prefTopic)
    cmd2arduino = cmd + ':' + str(msg.payload)+ endOfLine
    ser.write(cmd2arduino.encode('ascii','ignore'))
    # I immediately wait a moment for the arduino to respond
    time.sleep(sleepResponse)
    readArduinoAvailableMsg(ser)


# read all available messages from arduino
def readArduinoAvailableMsg(seri):
    while seri.inWaiting():
        # because of readline function, dont forget to open with timeout
        # serial communication generates many errors!
        #   this is a pb with devices that stays on all the time
        #   --> use try
        try :
            response = seri.readline().decode('utf-8', errors='ignore').replace('\n', '')
        except :
            response = ""
        
        #logp ("answer is:" + response, 'debug')
        mqTopic = 'notFound'
        prefTopic = 'noPref'
        for ipref in topicFromPref.keys():
            if response.startswith(ipref) :
                mqTopic   = topicFromPref[ipref]
                prefTopic = ipref
        #
        if response.startswith(prefTopic):
            # prefTopic: message to send to mqtt
            # I dont analyse those messages, I transmit to mqtt
            tags = response.replace(prefTopic, '',1).split(':')
            # security: I strip wildcard in topic before publishing
            topic = tags[0].replace('+', '').replace('#', '')
            pyTopic = mqTopic + topic
            if len(tags) > 1 :
                value = ':'.join(tags[1:])
            else :
                value = ''
            # trace
            logp('{} = {}'.format(pyTopic, value), 'to MQTT')
            # by default retain=True. But if topic ends with number, we assume there are numerous ones
            #   so they should not be retained by broker
            if re.search('/\d\d*', pyTopic):
               retain = False
            else:
               retain = True
            mqttc.publish(pyTopic, value, retain=retain)
        else :
            # I dont analyse, but I print
            logp (response, 'unknown from '+devSerial)


# check the correspondance  pref <---> topic   are correct
if checkTopicAndPref(prefFromTopic, topicFromPref) != 0:
    sys.exit()

# connection to arduino
ser = serial.Serial(devSerial, baudrate=baudRate, timeout=0.2)
logp (str(ser), 'info')
# when we open serial to an arduino, this reset the board; it needs ~3s
time.sleep(0.2)
#I empty arduino serial buffer
response = ser.readline()
logp ("arduino buffer garbage: " + str(response), 'info')
time.sleep(2)

# loop to get connection to arduino


#---------------------------------------------------
#                   mosquitto
#---------------------------------------------------

# read password from file  passMqtt.txt
authInFile = readDictPassFromFile(filePassMqtt)

# connection to mosquitto
# I write mqtt.MQTTv31, because broker may be V3.1  and not V3.1.1
mqttc = mqtt.Client("", True, None, mqtt.MQTTv31)
mqttc.on_message = on_message
mqttc.on_connect = on_connect
if (authInFile.has_key('username') and authInFile.has_key('password')) :
    mqttc.username_pw_set(authInFile["username"], authInFile["password"])

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



