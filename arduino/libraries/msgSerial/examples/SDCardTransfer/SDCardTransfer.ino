/*
  SD card test

 This example ...

 by ..
 */
// include the SD library:

#include <SPI.h>
//#include <SD.h>    // default SD card library
#include "SdFat.h"   // more complete SD card library:  https://github.com/greiman/SdFat
//SdFat SD;

#include "msgSerial.h"
#include "msgExampleFunction.h"

#include "msg2SDCard.h"

SerialListener serList(Serial);

Command cmdSD[] = {
    Command("openStay",     &srStayOpen,    "s,cc", "*,rwascet"),   // :bob.txt,r  filenameDOS8.3 (short names), openMode (read write... )
    Command("open",         &srPreOpen,     "s,cc", "*,rwascet"),   // :prepare to open at each read/write (it is closed immediately after)
    Command("close",        &srClose),          // pas de param
    Command("readln",       &srReadln),         // pas de param
    Command("writeln",      &srWriteln,     "s",    "*"),           // :a new lign to write (not \n terminated)
    Command("readNchar",    &srReadNchar,   "i",    "0-200"),       // :nbchar to read in a row
    Command("readNln",      &srReadNln,     "i",    "0-200"),       // :nb lines to read in a row
    Command("move",         &srMove,        "s"),       // :str2search
    Command("dump2",        &srDump2),                  // pas de param
    Command("ls",           &srLs,          "cc",   "rsda"),        // :sr  (recurse size ...)
    Command("rename",       &srRename,      "s,s"),     // :/adir/old,new
    Command("mkdir",        &srMkdir,       "s"),       // :/adir
    Command("rm",           &srRemove,      "s")        // :file.txt
};
CommandList cmdListSD("cmdSD", "SD+", SIZE_OF_TAB(cmdSD), cmdSD );

Command cmdSys[] = {
    Command("idSketch",  &sendSketchId),
    Command("idBuild",   &sendSketchBuild)
};
CommandList cmdListSys("cmdSys", "AT+", SIZE_OF_TAB(cmdSys), cmdSys );


// change this to match your SD shield or module;
// Arduino Ethernet shield: pin 4
// Adafruit SD shields and modules: pin 10
// Sparkfun SD shield: pin 8
// MKRZero SD: SDCARD_SS_PIN
const int chipSelect = 53;   // remplace 4

void setup() {
    // Open serial communications and wait for port to open:
    Serial.begin(38400);
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    // see if the card is present and can be initialized:
    if (!SD.begin(chipSelect)) {
        Serial.println("Card failed, or not present");
        // don't do anything more:
        return;
    }
    Serial.println("card initialized.");

    SD.ls("/", LS_R | LS_DATE | LS_SIZE);

    sketchInfo.setFileDateTime(F(__FILE__), F(__DATE__), F(__TIME__));

    serList.addCmdList(cmdListSD);
    serList.addCmdList(cmdListSys);

    // I send identification of sketch
    cmdListSys.readInternalMessage(F("idSketch"));
    cmdListSys.readInternalMessage(F("idBuild"));
}


void loop(void) {
    serList.checkMessageReceived();
}
