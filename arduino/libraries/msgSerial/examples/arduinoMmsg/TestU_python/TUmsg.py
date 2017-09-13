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

msgStartCmd='CM+'
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

scar("MultiValue")
scar("lit1/switch:ON")
time.sleep(1)
scar("sl13")      # error
scar("sl13:off")  # error
scar("sl13:OFF")

scar2("idSketch")
scar2("idBuild")
scar2("listCmd:,full")
scar2("listCmd:CM+,short")
scar2("listPin")
scar2("pinMode:9,i")
scar2("pinRead:9,d")
scar2("pinRead:15,a")
scar2("pinWrite:13,d,1")
scar("sl13:OFF")

ser.close()
