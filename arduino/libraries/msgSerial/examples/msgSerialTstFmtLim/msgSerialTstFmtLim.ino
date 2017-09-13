#include "Arduino.h"

#define LOG_DEBUG(str)   Serial.println(str)
#define LOG_ERROR(str)   msgSError(str)

//#include "SPI.h"
//#include "SdFat.h"
//#include "msg2SDCard.h"

#include "msgSerial.h"
#include "msgExampleFunction.h"

const int pinLed = 13;

// list of available commands (user) that the arduino will accept
int switchLed1(const CommandList& aCL, Command& aCmd, const String& dumb);

SerialListener serList(Serial);

// list of available commands (user) that the arduino will accept
//Command cmdUser[] = {
//    Command("S",             &sendMessageStatus),
//    Command("MultiValue",    &sendMultiValue),
//    Command("lit1/switch",   &switchLed1,       "s",        "ON|OFF"),
//    Command("sl13",          &switchLed13,      "s",        "ON|OFF")
//};
//CommandList cmdLUser("cmdUser", "CM+", SIZE_OF_TAB(cmdUser), cmdUser );

// list of available commands (system ctrl) that the arduino will accept
Command cmdSys[] = {
    Command("idSketch",  &sendSketchId),
    Command("idBuild",   &sendSketchBuild),
//    Command("listCmd",   &sendListCmd,      "s,s",      "*,short|full"),
//    Command("listPin",   &sendListPin),
//    Command("pinMode",   &cmdPinMode,       "i,s",      "0-30,o|i|ip"),
//    Command("pinRead",   &cmdPinRead,       "i,s",      "0-30,d|a"),
//    Command("close",        &srClose),          // pas de param
    Command("pinWrite",  &cmdPinWrite,      "i,s,d",    "0-30,d|a,*")
};
CommandList cmdLSys("cmdSys", "AT+", SIZE_OF_TAB(cmdSys), cmdSys );

// list of available commands (for tests) that the arduino will accept
Command cmdTest[] = {
    Command("S",             &sendMessageStatus),           // no fmt, no limit
    Command("t0lim",         &sendBackArg_s,      "s"),     // fmt, no limit
    Command("t0fmt",         &sendBackArg_s,      "",         "ON|OFF"),      // no fmt, with limit (not possible)
    Command("tbadlim",       &sendBackArg_s,      "s",        "ON|OFF,*"),    // limit incompatible with fmt
    Command("tls",           &sendBackArg_s,      "cc",       "rsda"),
    Command("tpinMode",      &sendBackArg_is,     "i,s",      "0-30,o|i|ip"),
    Command("tfval",         &sendBackArg_fs,     "f,s",      "0-100,*"),
    Command("tonoff",        &sendBackArg_s,      "s",        "ON|OFF")
};
CommandList cmdLTest("cmdTest", "t+", SIZE_OF_TAB(cmdTest), cmdTest );


/*---------------------------------------------------------------*/
/*                setup and loop function                        */
/*---------------------------------------------------------------*/

void setup()
{
    pinMode(pinLed, OUTPUT);

    Serial.begin(9600);

//    serList.addCmdList(cmdLUser);
    serList.addCmdList(cmdLSys);
    serList.addCmdList(cmdLTest);

    // I fill info on sketch
    sketchInfo.setFileDateTime(F(__FILE__), F(__DATE__), F(__TIME__));

    // I send identification of sketch
    cmdLSys.readInternalMessage(F("idSketch"));
    cmdLSys.readInternalMessage(F("idBuild"));
}
 
void loop()
{
  serList.checkMessageReceived();

  // I slow down Arduino
  delay(10);
}


/*---------------------------------------------------------------*/
/*                  list of user function                        */
/*---------------------------------------------------------------*/

// switch the led On or Off
int switchLed1(const CommandList& aCL, Command& aCmd, const String& sOnOff)
{
    ParsedCommand parsedCmd(aCL, aCmd, sOnOff);

    // verify that msg with arguments is OK with format and limit
    if (parsedCmd.verifyFormatMsg(aCmd, sOnOff) != ParsedCommand::NO_ERROR)
        return aCL.returnKO(aCmd, parsedCmd);

    return switchLed13(aCL, aCmd, sOnOff);
}

