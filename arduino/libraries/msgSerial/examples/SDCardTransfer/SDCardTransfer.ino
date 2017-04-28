/*
  SD card test

 This example ...

 by ..
 */
// include the SD library:

#include <SPI.h>
//#include <SD.h>
#include "SdFat.h"
SdFat SD;

#include "msgSerial.h"

#include "msg2SDCard.h"

SerialListener serList(Serial);

Command cmdUser[] = {
    Command("SendValue",     &sendMessageStatus),
    Command("srStayOpen",    &srStayOpen),    // :bob.txt,67  filenameDOS8.3 (short names), openMode (O_READ... specific to SdFat lib)
    Command("srPreOpen",     &srPreOpen),     // :prepare to open at each read/write (it is closed immediately after)
    Command("srClose",       &srClose),   // pas de param
    Command("srReadln",      &srReadln),  // pas de param
    Command("srWriteln",     &srWriteln), // :nouvelle ligne
    Command("srReadNchar",   &srReadNchar),   // :nbchar
    Command("srMove",        &srMove),    // :str2search
    Command("srDump",        &srDump),    // pas de param
    Command("srLs",          &srLs),      // :15  donner les options du ls
    Command("srRename",      &srRename),  // :/adir/old,new
    Command("srMkdir",       &srMkdir),   // :/adir
    Command("srRemove",      &srRemove),  // :file.txt
    Command("S",             &sendMessageStatus)
};
CommandList cmdListUser("cmdUser", "CM+", SIZE_OF_TAB(cmdUser), cmdUser );

Command cmdSys[] = {
    Command("idSketch",  &sendSketchId),
    Command("idBuild",   &sendSketchBuild)
};
CommandList cmdListSys("cmdSys", "CS+", SIZE_OF_TAB(cmdSys), cmdSys );


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

    serList.addCmdList(cmdListUser);
    serList.addCmdList(cmdListSys);

    sendSketchId("");
    sendSketchBuild("");
}


void loop(void) {
    serList.checkMessageReceived();
}
