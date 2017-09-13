#!/usr/bin/python

from __future__ import print_function
import serial
import time

# empty serial buffer
def emptyRx(ser):
   response = ser.read(100)
   while (len(response) >0 ):
      print (response, end='')
      response = ser.read(100)
   if (not response.endswith('\n')) and (len(response) >0):
      print('')

# use to sort log messages
def logp (msg, gravity='trace'):
   print('['+gravity+']' + msg)

msgStartCmd='t+'
msgStartCmd2='AT+'
msgEnd='\n'

devSerial='/dev/ttyACM0'


def sendCmdArd(aCmd):
    cmd2arduino = msgStartCmd + aCmd + msgEnd
    ser.write(cmd2arduino)

def sendCmdArdReceive(aCmd):
    sendCmdArd(aCmd)
    emptyRx(ser)

def scar(aCmd, prefix=msgStartCmd):
   cmd2arduino = prefix + aCmd + msgEnd
   ser.write(cmd2arduino)
   emptyRx(ser)

def scar2(aCmd, prefix=msgStartCmd2):
   cmd2arduino = prefix + aCmd + msgEnd
   ser.write(cmd2arduino)
   emptyRx(ser)


ser = serial.Serial(devSerial, baudrate=38400, timeout=0.2, writeTimeout=0.2)
# waiting 1s for arduino reinit
time.sleep(1)
emptyRx(ser)

ser.write("t+S\n")
emptyRx(ser)
sendCmdArd("S");emptyRx(ser)
scar("S")
scar("tonoff:ON")
time.sleep(1)
scar("tonoff:OFF")

scar("t0lim:fmt_seul")
scar("tls:rs")
scar("tfval:1.2,rien")
scar("tpinMode:2,i")

print("Next commands are impossible, they generate errors")
time.sleep(1)

ser.write("not a command\n")
emptyRx(ser)

ser.write("no header S\n")
emptyRx(ser)

print("cmd is not already processed because there is no newline")
print("Yet emptyRx wait 0.2s for a response !")
time.sleep(0.2)
emptyRx(ser)
print("cmd is processed later, when buffer is emptied")

ser.write("t+not_a_cmd\n")
emptyRx(ser)

print("arg missing")
scar("t0lim")
print("0 fmt with lim is not possible")
scar("t0fmt")
scar("t0fmt:fake")
scar("tbadlim:ON")
scar("tbadlim:ON,rien")
scar("tls:t")
scar("tls:rst")
scar("tls:tr")
scar("tls")

print("bad type")
scar("tpinMode:f,o")

scar("tfval:notFloat,rien")


ser.close()
