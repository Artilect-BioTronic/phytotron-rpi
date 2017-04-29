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

def scar(aCmd):
    sendCmdArd(aCmd)
    emptyRx(ser)

def scar2(aCmd):
    ser.write(msgStartCmd2 + aCmd + msgEnd)
    emptyRx(ser)


ser = serial.Serial(devSerial, baudrate=38400, timeout=0.2, writeTimeout=0.2)
# waiting 1s for arduino reinit
time.sleep(1)
emptyRx(ser)

ser.write("CM+srLs:" + "15\n")
emptyRx(ser)

# mode=71 , read / write / append / create
sendCmdArd("srPreOpen:" + "test.txt,71");emptyRx(ser)
sendCmdArd("srWriteln:testing 1, 2, 3.");emptyRx(ser)
scar("srWriteln:line2 is here")
scar("srWriteln:The line 3 is plain")
ser.write("CM+srClose\n");emptyRx(ser)
scar("srLs:13")

# mode=1, open readonly. the file stays open
sendCmdArd("srStayOpen:" + "test.txt,1")
ser.write("CM+srReadln\n")
scar("srWriteln:cannot write on readonly")
sendCmdArd("srReadln");emptyRx(ser)
scar("srClose")

# mode=1, open readonly
scar("srPreOpen:test.txt,1")
ser.write("CM+srReadln\n")
ser.write("CM+srMove:The line 3");emptyRx(ser)
ser.write("CM+srReadNchar:30\n");emptyRx(ser)
scar("srReadln")
scar("srReadNchar:30")
scar("srClose")

scar("srLs:13")
scar("srMkdir:Folder1")
scar("srPreOpen:Folder1/file1.txt,67")
print("Open cannot  create file at open  in subdirectory !!")
scar("srClose")
scar("srPreOpen:file1.txt,67")
scar("srPreOpen:file2.txt,67")
scar("srClose")
scar("srLs:13")
scar("srRename:file2.txt,Folder1/file3.txt")
scar("srLs:13")
scar("srRemove:file1.txt")
scar("srRemove:Folder1/file3.txt")
scar("srRemove:Folder1/")
scar("srLs:13")


ser.close()
