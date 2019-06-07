#!/usr/bin/python

from __future__ import print_function
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import serial
import time
import sys, getopt
import ast
import os, glob

#repSketch='/opt/openhab2/conf/Python_MQTT2Arduino'
#repTmp='/media/ramdisk/openhab/logPython'
repSketch=os.getcwd()
fileNameListSketch = repSketch + '/listSketch.txt'
repTmp=repSketch

hostMQTT='localhost'
portMQTT=1883
devSearchString='/dev/tty[UA]*'
sleepBetweenLoop=2

myTopicWatch='/domotique/watch/'
topFromSys='sys/'
reconnectTopic='reconnect'

prefAT='AT+'
cmdIdSketch='idSketch'
endOfLine='\n'
msg2py='AT'

logfile=sys.stdout

startedSerial2MQTT = {}
lastListDev = []


# use to sort log messages
def logp (msg, gravity='trace'):
	print('['+gravity+']' + msg, file=logfile)

# log file must not grow big
# I need to overwrite it often
def reOpenLogfile(logfileName):
	"re open logfile, I do it because it must not grow big"
	global logStartTime, logfile, startedSerial2MQTT
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
			logp('list of started devices:' + str(startedSerial2MQTT), 'info')
		except IOError:
			print('[error] could not open logfile:' + logfileName)
			sys.exit(3)
	else :
		print('I cant re open logfile. name is empty')



def read_args(argv):
	# optional args have default values above
	global logfile, repTmp
	logfileName = ''
	try:
		opts, args = getopt.getopt(argv,"hr:l:",["reptmp=","logfile="])
	except getopt.GetoptError:
		print ('watchConnectPython2Arduino.py -r <reptmp> -l <logfile>')
		sys.exit(2)
	for opt, arg in opts:
		if opt == '-h':
			print ('watchConnectPython2Arduino.py -r <reptmp> -l <logfile>')
			sys.exit()
		elif opt in ("-l", "--logfile"):
			if (arg.count('stdout') > 0) :
				logfileName = ''
				logfile = sys.stdout
				print ('logfile = ' + str(logfile))
			else:
				logfileName = arg
		elif opt in ("-r", "--reptmp"):
			repTmp = arg
	logp('repTmp is '+ repTmp, 'debug')
	logp('logfile is '+ logfileName, 'debug')
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
	"read filename and append/update valid dict lines in returned dict"
	logp('reading list of arguments for sketch in file : ' + fileName)
	dSketch = updateDictFromFile(fileName)
	return dSketch

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

def readDictFromFileOld(fileName, dSketch):
	"read filename and append valid dict lines in returned dict"
	try:
		fileListSketch = open(fileName, 'r')
	except:
		logp('could not open dictionnary file: ' + fileName, 'error')
		return {}
	#
	strLines = fileListSketch.readlines()
	fileListSketch.close()
	dSketch = {}
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
	fileListSketch.close()
	return dSketch

def launchSerial2MQTT(da, arepTmp,adevSerial):
	"Launches the python script serial2MQTTduplex.py thanks to the dictionnary of arg"
	# test that it has all required arg
	for keyNeeded in ("logfile","broker","topicFromPref","prefFromTopic"):
		if not (da.has_key(keyNeeded)):
			logp('I refuse to launch serial2MQTTduplex ...', 'error')
			logp(' because I cannot find key ' + keyNeeded, 'error')
			return
	cmd = repSketch+ '/serial2MQTTduplex.py'+ ' -l '+arepTmp+'/' + da['logfile'] \
	    + ' -b '+da['broker'] + ' -d ' + adevSerial + ' -r 9600' \
	    + ' -t "'+str(da['topicFromPref'])+'"' + ' -u "'+str(da['prefFromTopic'])+'"' +' &'
	logp('launching with cmd: ' + cmd , 'info')
	os.system(cmd)


def askIdSketchSerial(adevSerial):
	"try to know the id of the arduino sketch linked on serial adevSerial"
	# I get rid of AMA0
	if (adevSerial.count('AMA') > 0):
		logp('I dont look for sketch on AMA serial', 'info')
		return ' '
	#
	logp('trying to recognize arduino sketch on serial:' + adevSerial , 'info')
	rIdSketch = ''
	try:
		ser = serial.Serial(adevSerial, baudrate=9600, timeout=0.2, writeTimeout=0.2)
		logp (str(ser), 'info')
		time.sleep(0.2)
		#
		#I empty arduino serial buffer
		response = ser.read(100)
		logp ("arduino buffer garbage: " + str(response), 'info')
		#
		# when we open serial to an arduino, this reset the board; it needs ~3s
		time.sleep(3)
	except:
			logp ('could not ask idSketch to serial '+adevSerial , 'warning')
			return ' '
	#
	try:
		# I send a sys cmd to ask for id
		cmd = (prefAT + cmdIdSketch + endOfLine)
		ser.write(cmd)
		logp (cmd)
	except:
		logp('may be not a serial com: '+adevSerial, 'info')
		ser.close()
		return ' '
	# I give time to respond
	time.sleep(0.2)
	# I sort the response, I will get rid of unused messages
	while ser.inWaiting():
		# because of readline function, dont forget to open with timeout
		response = ser.readline().replace('\n', '')
		#logp ("answer is:" + response, 'debug')
		if response.rfind(prefAT + cmdIdSketch) >= 0:
			# prefAT prefixe for sys cmd
			logp ('prefAT '+response, 'info')
			nameInd = response.rfind(':')	
			if (nameInd >= 0):
				rIdSketch = response[nameInd+1:]
				ser.close()
				return rIdSketch
		else :
			# I dont analyse, but I print
			logp (response, 'unknown from '+adevSerial)
	# not found, I must not return empty value !
	return ' '

def giveUpdateListSerialDev(oldListDev, genericLsNameDev):
	"list all serial dev connected; updates oldListDev, return list of new dev and dead dev"
	newListDev = glob.glob(genericLsNameDev)
	newListDev.sort()
	lDevNew = []; lDevDead = []
	if (cmp(oldListDev,newListDev) != 0):
		# I look for new connected device
		for dev in newListDev:
			if oldListDev.count(dev) == 0:
				lDevNew.append(dev)
		# I look for dead (not) connected device
		for dev in oldListDev:
			if newListDev.count(dev) == 0:
				lDevDead.append(dev)
		logp('I have discovered changes in serial devices:  new: '+ str(lDevNew)+ ' and dead: '+str(lDevDead), 'info')
	return [newListDev, lDevNew, lDevDead ]


# mosquitto part
# --------------

def treatSysMsg(msgCmd, msgVal):
	"treat systeme msg coming from mosquitto"
	if (msgCmd.count(reconnectTopic) > 0):
		# the name of /dev/tty to reconnect is in msgVal
		if (len(msgVal) == 0):
			logp('name of device to reconnect is empty', 'warn')
			return
		# I take off dev from list
		if (lastListDev.count(msgVal) > 0):
			lastListDev.remove(msgVal)
	# after removal from lastListDev, the script will see it exists and 
	#   it will try to reconnect it

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, rc):
	print("Connected to mosquitto with result code "+str(rc))
	# Subscribing in on_connect() means that if we lose the connection and
	# reconnect then subscriptions will be renewed.
	mqttc.subscribe(myTopicWatch+ topFromSys +'#')
	mqttc.message_callback_add(myTopicWatch+ topFromSys+ '#', on_message_myTopicSys)

# The callback for when a PUBLISH message is received from the server.
# usually we dont go to  on_message
#  because we go to a specific callback that we have defined for each topic
def on_message(client, userdata, msg):
	"default callback for message. no action provided"
	logp('not used msg:' +msg.topic+" : "+str(msg.payload), 'info')

# The callback for when a PUBLISH message is received from the server.
# usually  we go to a specific callback that we have defined for each topic
def on_message_myTopicSys(client, userdata, msg):
	logp("spec callbackSys,"+msg.topic+":"+str(msg.payload), 'info')
	cmdMsg = msg.topic.replace(myTopicWatch, '').replace(topFromSys, '').replace('#','')
	treatSysMsg(cmdMsg, msg.payload)


#  main part of script
#  -------------------


# connection to mosquitto
mqttc = mqtt.Client("", True, None, mqtt.MQTTv31)
mqttc.on_message = on_message
mqttc.on_connect = on_connect

cr = mqttc.connect(hostMQTT, port=portMQTT, keepalive=60, bind_address="")
mqttc.loop_start()


while True:
	# check if the list of serial has changed
	listDevNew=[]; listDevDead=[]
	[lastListDev, listDevNew, listDevDead] = giveUpdateListSerialDev(lastListDev, devSearchString)
	#
	# we remove dead dev from list of active dev
	for dev in listDevDead:
		if ( startedSerial2MQTT.has_key(dev) ):
			del startedSerial2MQTT[dev]
	#
	# if we detect new connected device, we try to connect python com script
	if (len (listDevNew) > 0):
		# we update list of scripts
		dSketch = readListSketchFromFile(fileNameListSketch)
		if (len(dSketch) == 0):
			logp('empty list of sketch configuration in file: ' + fileNameListSketch , 'error')
			exit(-1)
		# we try to connect python com script to new devices
		for ndl in listDevNew:
			logp('trying to recognize and connect new serial device: '+ ndl, 'info')
			idSketch = askIdSketchSerial(ndl)
			if dSketch.has_key(idSketch):
				launchSerial2MQTT(dSketch[idSketch], repTmp, ndl )
				# I am trusty, I immediately add it to active device list
				startedSerial2MQTT.update({ndl:idSketch})
			else:
				logp('I dont have this arduino id sketch in my listing file:' + idSketch, 'error')
	#
	# if there was an update, I log the new list of started dev
	if (len (listDevNew) > 0  or len (listDevDead) > 0):
		logp('list of started device:' + str(startedSerial2MQTT), 'info')
	#
	#check size, logfile should not get too big
	checkLogfileSize(logfile)
	#
	time.sleep(sleepBetweenLoop)


