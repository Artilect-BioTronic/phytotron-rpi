#!/usr/bin/python

# on pourrait tester avec la lib arduino d ecriture ""

# l arduino risque d etre mega

from __future__ import print_function
import serial
import time

def emptyRx(ser):
    response = ser.read(100)
    while (len(response) >0 ):
        print (response)
        response = ser.read(100)

# use to sort log messages
def logp (msg, gravity='trace'):
   print('['+gravity+']' + msg)

msgStartAT='CM+'
msgStartDO='DO+'
msgEnd='\n'

devSerial='/dev/ttyACM0'
bd=9600


def sendCmdArd(aCmd):
   cmd2arduino = msgStartAT + aCmd + msgEnd
   ser.write(cmd2arduino)
   emptyRx(ser)

def sendGetArd(aCmd):
   # I send the cmd
   sendCmdArd(aCmd)
   #
   # I have to get back response
   # I wait, I give serial some time to transmit 50 char
   time.sleep(0.2)
   timeStartWait = time.time()
   while ( (not ser.inWaiting()) and (time.time() - timeStartWait < 1) ):
      print('waiting:%d' %  ser.inWaiting())
      time.sleep(0.02)
   respLoad = ''
   cr = 0
   # I listen to buffer
   while ser.inWaiting():
      # because of readline function, dont forget to open with timeout
      responseRaw = ser.readline()
      if responseRaw.startswith(msgStartAT) :
         responseRaw2 = responseRaw.replace(msgEnd, '').replace(msgStartAT, '')
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
      elif responseRaw != '\n' :   # I skip empty lines
         # I dont analyse, but I print
         logp (responseRaw.replace('\n',''), 'unknown from '+devSerial)
   return ['', -1]

def saveRx(ser, fileSaveName):
    fileSave = open(fileSaveName, "w", 1)
    response = ser.read(100)
    while (len(response) >0 ):
        print ("receiving " + str(len(response)) + "char")
        fileSave.write(response)
        response = ser.read(100)
    fileSave.close()


ser = serial.Serial(devSerial, baudrate=bd, timeout=0.2, writeTimeout=0.2)
# waiting 2s for arduino reinit
time.sleep(1)
emptyRx(ser)

[response, cr] = sendGetArd("srOpen:" + fileSDN + ",1")
[response, cr] = sendGetArd("srLs:" + "15")

#ser.write("AT+pinWrite:13,1,1""\n")
ser.write("CM+SendValue""\n")
emptyRx(ser)

emptyRx(ser)

ser.close()


sys.exit()

serial2MQTTduplex.py -d "/dev/ttyAMA0" -t "/phytotron/" -u "/phytotron/"
/home/pi/python/serial2MQTTduplex.py -d "/dev/ttyAMA0" -t "/phytotron/" -u "/phytotron/" &
