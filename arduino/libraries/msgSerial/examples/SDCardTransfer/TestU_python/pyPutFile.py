#!/usr/bin/python

from __future__ import print_function
import paho.mqtt.client as mqtt
import paho.mqtt.publish as publish
import serial
import time
import sys, getopt


devSerial='/dev/ttyACM0'   # serial port the arduino is connected to
fileSDN = "COM.CSV"
fileHDN = "fake"
fileHD = sys.stdout
moveTo = ''
moveTo2 = ''

cmdSendValue='SendValue'


# serial msg to arduino begin  with  msgStartAT / msgStartDO and end with endOfMsg
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



# open file on Hard Disk
# TO DO put fileHD as return
def openHDFile(afileHDName):
   "re open logfile, I do it because it must not grow big"
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


def emptyRx(ser):
   response = ser.read(100)
   while (len(response) >0 ):
      print (response, end='')
      response = ser.read(100)
   print('')

def sendCmdArd(aCmd):
   cmd2arduino = msgStartAT + aCmd + msgEnd
   ser.write(cmd2arduino)

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


# just to play
sendCmdArd("SendValue")
emptyRx(ser)


# open modes are different from basic SD.h library
# open modes are (0X08 to 0X01): O_SYNC | O_APPEND | O_WRITE | O_READ
# open modes are (0X80 to 0X10): O_EXCL | O_CREAT | O_AT_END | O_TRUNC
# open with R/W, create if necessary
[response, cr] = sendGetArd("srStayOpen:" + fileSDN + ",67")
if cr == -1 :
   print ('open : ', fileSDN, ' failed', '\nbye')
   sys.exit(-1)

print ('open : ', fileSDN , ':', cr)

# We move to a special position in file
# we start by moving to begin of file (open as write provoke append at end)
if (moveTo != '') or (moveTo2 != '') :
   [response, cr] = sendGetArd("srMove:" + "BEGIN")

if moveTo != '':
   [response, cr] = sendGetArd("srMove:" + moveTo)

if moveTo2 != '':
   [response, cr] = sendGetArd("srMove:" + moveTo2)


try:
    fileHD = open(fileHDN, 'r')
except:
    logp('could not open file on HD: ' + fileHDN, 'error')
    sys.exit(-2)
#
strLines = fileHD.readlines()
fileHD.close()
for strl in strLines:
    [response, cr] = sendGetArd("srWriteln:" + strl.replace('\n',''))
    #logp('writing:' + strl, 'info')
    if cr != 0:
        print('response and cr !=0:%s, %d' % (response,cr))
        break


[response, cr] = sendGetArd("srClose")
print ('close, cr: ', cr)

if fileHDN != '<stdout>' :
   fileHD.close()

ser.close()

sys.exit(0)

#===============================

emptyRx(ser)

emptyRx(ser)
ser.write("CM+srOpen:" + fileSD +"\n")
emptyRx(ser)
ser.write("CM+srReadln""\n")
emptyRx(ser)
ser.write("CM+srClose""\n")
emptyRx(ser)

ser.close()



