#!/usr/bin/python

from __future__ import print_function
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import serial
import time
import sys, getopt


devSerial='/dev/ttyACM0'   # serial port the arduino is connected to
fileSDN = "COM.CSV"
fileHDN = ""
fileHD = sys.stdout
moveTo = ''
moveTo2 = ''

cmdSendValue='SendValue'


# serial msg to arduino begin  with  msgStartAT / msgStartDO and end with msgEnd
msgStartAT='CM+'
msgStartDO='DO+'
msgEnd='\n'
# arduino responds with those 2 kinds of messages
msg2py='2py'
msg2mqtt='2mq'


# use to sort log messages
def logp (msg, gravity='trace'):
	print('['+gravity+']' + msg)


def read_args(argv):
	# optional args have default values above
	global devSerial, fileSDN, fileHDN, moveTo, moveTo2
	try:
		opts, args = getopt.getopt(argv,"hd:f:g:m:n:",["devserial=","fileInSDCard=","fileOnHD=","moveTo=","moveTo2="])
	except getopt.GetoptError:
		print ('pyGetFile.py  -d <devserial> -f <fileInSDCard> -g <fileOnHD> -m <moveTo> -n <moveTo2>')
		sys.exit(2)
	for opt, arg in opts:
		if opt == '-h':
			print ('pyGetFile.py  -d <devserial> -f <fileInSDCard> -g <fileOnHD> -m <moveTo> -n <moveTo2>')
			sys.exit()
		elif opt in ("-d", "--devserial"):
			devSerial = arg
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
	logp('devserial is '+ devSerial, 'debug')
	logp('moveTo is '   + moveTo, 'debug')
	logp('moveTo2 is '  + moveTo2, 'debug')


if __name__ == "__main__":
	read_args(sys.argv[1:])


# The callback for when a PUBLISH message is received from the server.
# usually  we go to a specific callback that we have defined for each topic
def on_message_myTopicSys(client, userdata, msg):
	logp("spec callbackSys,"+msg.topic+":"+str(msg.payload), 'info')
	cmd = msg.topic.replace(myTopic1, '').replace(topFromSys, '').replace('#','')
	cmd2arduino = msgStartAT + cmd + ':' + str(msg.payload)+ msgEnd
	ser.write(cmd2arduino)


# open file on Hard Disk
# TO DO put fileHD as return
def openHDFile(afileHDName):
	global fileHD
	#
	if (afileHDName != '') and (afileHDName != '<stdout>') :
		try:
			# I close file if needed
			if ( not fileHD.closed ) and (fileHD.name != '<stdout>') :
				fileHD.close()
			# file will be overwritten
			if (afileHDName != '<stdout>') :
				fileHD = open(afileHDName, "w", 1)
			logp('open fileHD' + time.asctime(time.localtime(time.time())), 'info')
		except IOError:
			print('[error] could not open fileHD:' + afileHDName)
			sys.exit(3)
	else :
		print('I cant open destination file. name is empty')


# read all available messages from arduino
def readArduinoAvailableMsg(seri):
    while seri.inWaiting():
        # because of readline function, dont forget to open with timeout
        response = seri.readline().replace('\n', '')
        #logp ("answer is:" + response, 'debug')
        tags = response.split(';')
        if tags[0] == msg2mqtt:
            # msg2mqtt: message to send to mqtt
            # I dont analyse those messages, I transmit to mqtt
            (topic, value) = tags[1].split(':')
            pyTopic = myTopic1 + topFromPy + topic
            # trace
            logp('{} = {}'.format(pyTopic, value), 'to MQTT')
            mqttc.publish(pyTopic, value, retain=True)
        elif tags[0] == msg2py:
            # msg2py: message 2 python only
            # python use this to check connection with arduino
            logp ('msg2py '+response, 'info')
        else :
            # I dont analyse, but I print
            logp (response, 'unknown from '+devSerial)


def emptyRx(ser):
   response = ser.read(100)
   while (len(response) >0 ):
      print (response, end='')
      response = ser.read(100)
   print('')

def sendCmdArd(aCmd):
   cmd2arduino = msgStartAT + aCmd + msgEnd
   ser.write(cmd2arduino)

def parseCmdAnswer(aCmdAnswer):
    if (not aCmdAnswer.startswith(msgStartAT)) :
        return ['', -1]
    else :
       # we take off  msgStartAT  and  msgEnd
       responseRaw  = aCmdAnswer[:len(aCmdAnswer)-len(msgEnd)]
       responseRaw2 = responseRaw.replace(msgStartAT, '')
       tags = responseRaw2.split(':')
       respCmd = tags[0]
       if len(tags) > 1:
          respLoad = ':'.join(tags[1:])
       else :
          respLoad = ''
       if respCmd.endswith('KO'):
          cr = -1
       else :
          cr = 0
       return [respLoad, cr]

def isRespCplt(aResp) :
    """we check if response is complete
    CM+srCmd:arg\n                     complete
    CM++20,srCmd2:load\n               not complete
    CM++20,srCmd2:load\nabcdefgh\n     complete
    """
    if (not aResp.startswith(msgStartAT)) :
        return False
    resp2 = aResp.replace(msgStartAT, '')
    #
    if (not resp2.startswith('+')) :
        return True
    resp3 = resp2.replace('+', '')
    #
    tags = resp3.split(',')
    sizeResp = tags[0]
    if (len(tags) == 1) :
        return True       # the format is weird but I say it is complete
    resp4 = ','.join(tags[1:])
    tags = resp4.split(':')
    if (len(tags) == 1) :
        return True       # the format is weird but I say it is complete
    resp5 = ':'.join(tags[1:])
    if (len(resp5) - len(msgEnd) < int(sizeResp)) :
        return False
    else :
        return True

def sendGetArd(aCmd):
   # I send the cmd
   sendCmdArd(aCmd)
   #
   # I have to get back response
   # I wait, I give serial some time to transmit 50 char
   time.sleep(0.02)
   timeStartWait = time.time()
   while ( (not ser.inWaiting()) and (time.time() - timeStartWait < 1) ):
      print('waiting:%d' %  ser.inWaiting())
      time.sleep(0.02)
   #
   respLoad = ''
   respCplt = ''
   isRespInSeveral = False
   cr = 0
   # I listen to buffer
   while ser.inWaiting():
      # because of readline function, dont forget to open with timeout
      responseRaw = ser.readline()
      if isRespInSeveral :
          respCplt = respCplt + responseRaw
      else :
          respCplt = responseRaw
      #
      if respCplt.startswith(msgStartAT) :
         if isRespCplt(respCplt) :
             [respLoad, cr] = parseCmdAnswer(respCplt)
             return [respLoad, cr]
         else :
             isRespInSeveral = True
             time.sleep(0.05)
      elif respCplt != '\n' :   # I skip empty lines
         # I dont analyse, but I print
         logp (respCplt.replace('\n',''), 'unknown from '+devSerial)
   #
   return ['', -1]


# connection to arduino
# I use 9600, because I had many pb with pyserial at 38400 !!!
ser = serial.Serial(devSerial, baudrate=38400, timeout=0.2, writeTimeout=0.2)
logp (str(ser), 'info')
# when we open serial to an arduino, this reset the board; it needs ~3s
time.sleep(0.2)
#I empty arduino serial buffer
response = ser.readline()
logp ("arduino buffer garbage: " + str(response), 'info')
time.sleep(1)


openHDFile(fileHDN)
print ('file: '+ fileHDN + ' opened')

[response, cr] = sendGetArd("srPreOpen:" + fileSDN + ",1")
if cr == -1 :
   print ('open : ', fileSDN, ' failed', '\nbye')
   sys.exit()

print ('open : ', fileSDN , ':', cr)

if moveTo != '':
   [response, cr] = sendGetArd("srMove:" + moveTo)

nbChar = 30
[response, cr] = sendGetArd("srReadNchar:%d" % nbChar)
while cr == 0 :
   print(response, end='', file=fileHD)
   [response, cr] = sendGetArd("srReadNchar:%d" % nbChar)

[response, cr] = sendGetArd("srClose")
print ('close, cr: ', cr)

if fileHDN != '<stdout>' :
   fileHD.close()

ser.close()

sys.exit(0)

#===============================

