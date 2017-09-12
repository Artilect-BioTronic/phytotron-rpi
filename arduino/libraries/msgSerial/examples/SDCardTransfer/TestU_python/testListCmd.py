#!/usr/bin/python

# on pourrait tester avec la lib arduino d ecriture ""

# l arduino risque d etre mega

from __future__ import print_function
import serial
import time

msgStartT='t+'
msgStartAT='AT+'
msgEnd='\n'

devSerial='/dev/ttyACM0'
bd=38400
fileSD = "fake"


# use to sort log messages
def logp (msg, gravity='trace'):
   print('['+gravity+']' + msg)

def emptyRx(ser):
    response = ser.read(100)
    while (len(response) >0 ):
        print (response, end='')
        response = ser.read(100)
    print ('')  # end of line

def sendCmdArd(aCmd):
   cmd2arduino = msgStartAT + aCmd + msgEnd
   ser.write(cmd2arduino)

def scar(aCmd, prefix=msgStartT):
   cmd2arduino = prefix + aCmd + msgEnd
   ser.write(cmd2arduino)
   emptyRx(ser)

def scar2(aCmd, prefix=msgStartAT):
   cmd2arduino = prefix + aCmd + msgEnd
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


ser = serial.Serial(devSerial, baudrate=bd, timeout=0.2, writeTimeout=0.2)
# waiting 2s for arduino reinit
time.sleep(1)
emptyRx(ser)

[response, cr] = sendGetArd("open:" + fileSD + ",1")
[response, cr] = sendGetArd("ls:" + "rsda")

ser.write("CM+SendValue""\n")
emptyRx(ser)
ser.write("CM+open:" + fileSD +"\n")
emptyRx(ser)
ser.write("CM+readln""\n")
emptyRx(ser)
ser.write("CM+readln""\n")
ser.write("CM+readln""\n")
ser.write("CM+readln""\n")
ser.write("CM+close""\n")

emptyRx(ser)

ser.close()


sys.exit()

# old
serial2MQTTduplex.py -d "/dev/ttyAMA0" -t "/phytotron/" -u "/phytotron/"
/home/pi/python/serial2MQTTduplex.py -d "/dev/ttyAMA0" -t "/phytotron/" -u "/phytotron/" &
# new
serial2MQTTduplex.py -r 38400 -d /dev/ttyACM0 -t '{"CM+":"phytotron/SDlog/receive/"}' -u '{"phytotron/SDlog/send/":"SD+"}' &
serial2MQTTduplex.py -r 38400 -d /dev/ttyACM0 -t '{"CM+":"phytotron/py/","AT+":"phytotron/py/" }' -u '{"phytotron/oh/":"CM+", "phytotron/admini/":"AT+"}' &
./serial2MQTTduplex.py -l "./logSerial2MQTT.txt" -r 115200 -d "/dev/ttyAMA0" -t '{"CM+":"phytotron/arduMain/py/","AT+":"phytotron/admini/py/" }' -u '{"phytotron/arduMain/oh/":"CM+", "phytotron/admini/oh":"AT+"}' &
./serial2MQTTduplex.py -l "./logSerial2MQTT.txt" -r 115200 -d "/dev/ttyAMA0" -t '{"CM+":"phytotron/arduMain/py/","AT+":"phytotron/admini/py/", "SD+":"phytotron/SDlog/receive/" }' -u '{"phytotron/arduMain/oh/":"CM+", "phytotron/admini/oh/":"AT+", "phytotron/SDlog/send/":"SD+"}' &

