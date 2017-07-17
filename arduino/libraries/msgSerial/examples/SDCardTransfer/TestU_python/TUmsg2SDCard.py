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

ser.write("SD+ls:" + "rsa\n")
emptyRx(ser)

# mode=71 , read / write / append / create
sendCmdArd("open:" + "test.txt,rwac");emptyRx(ser)
sendCmdArd("writeln:testing 1, 2, 3.");emptyRx(ser)
scar("writeln:line2 is here")
scar("writeln:The line 3 is plain")
ser.write("SD+close\n");emptyRx(ser)
scar("ls:rs")

# mode=1, open readonly. the file stays open
sendCmdArd("openStay:" + "test.txt,1")
ser.write("SD+readln\n")
scar("writeln:cannot write on readonly")
sendCmdArd("readln");emptyRx(ser)
scar("close")

# mode=1, open readonly
scar("open:test.txt,r")
scar("move:This line wont be found")
ser.write("SD+readln\n")
ser.write("SD+move:The line 3");emptyRx(ser)
ser.write("SD+readNchar:30\n");emptyRx(ser)
scar("readln")
scar("readNchar:30")
scar("close")

scar("ls:rs")
scar("mkdir:Folder1")
scar("open:Folder1/file1.txt,rwc")
print("Open cannot  create file at open  in subdirectory !!")
scar("close")
scar("open:file1.txt,rwc")
scar("open:file2.txt,rwc")
scar("close")
scar("ls:rs")
scar("rename:file2.txt,Folder1/file3.txt")
scar("ls:rs")
scar("rm:file1.txt")
scar("rm:Folder1/file3.txt")
print("to remove a directory, I must end the name with /")
scar("rm:Folder1/")
scar("ls:rs")


ser.close()
