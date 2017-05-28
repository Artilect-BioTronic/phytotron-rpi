/*
  SD card test

 This example ...

 by ..
 */
// include the SD library:

#include <SPI.h>
//#include <SD.h>
#include "SdFat.h"
//SdFat SD;

#include "msgSerial.h"

#include "msg2SDCard.h"

SerialListener serList(Serial);

Command cmdUser[] = {
    Command("openStay",     &srStayOpen),    // :bob.txt,67  filenameDOS8.3 (short names), openMode (O_READ... specific to SdFat lib)
    Command("open",         &srPreOpen),     // :prepare to open at each read/write (it is closed immediately after)
    Command("close",        &srClose),   // pas de param
    Command("readln",       &srReadln),  // pas de param
    Command("writeln",      &srWriteln), // :nouvelle ligne
    Command("readNchar",    &srReadNchar),   // :nbchar
    Command("move",         &srMove),    // :str2search
    Command("dump2",        &srDump2),   // pas de param
    Command("ls",           &srLs),      // :15  donner les options du ls
    Command("rename",       &srRename),  // :/adir/old,new
    Command("mkdir",        &srMkdir),   // :/adir
    Command("rm",           &srRemove)   // :file.txt
};
CommandList cmdListUser("cmdUser", "SD+", SIZE_OF_TAB(cmdUser), cmdUser );

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

    serList.addCmdList(cmdListUser);
    serList.addCmdList(cmdListSys);

    sendSketchId("");
    sendSketchBuild("");
}


void loop(void) {
    serList.checkMessageReceived();
}
