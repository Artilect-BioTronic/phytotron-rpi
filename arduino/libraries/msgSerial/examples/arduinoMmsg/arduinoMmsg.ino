#include "Arduino.h"

#define LOG_DEBUG(str)   Serial.println(str)
#define LOG_ERROR(str)   msgSError(str)

#include "msgSerial.h"
#include "msgExampleFunction.h"

const int pinLed = 13;

// list of available commands (user) that the arduino will accept
int switchLed1(const CommandList& aCL, Command& aCmd, const String& dumb);

SerialListener serList(Serial);

// list of available commands (user) that the arduino will accept
Command cmdUser[] = {
    Command("S",             &sendMessageStatus),
    Command("MultiValue",    &sendMultiValue),
    Command("lit1/switch",   &switchLed1,       "s",        "ON|OFF"),
    Command("sl13",          &switchLed13,      "s",        "ON|OFF")
};
CommandList cmdLUser("cmdUser", "CM+", SIZE_OF_TAB(cmdUser), cmdUser );

// list of available commands (system ctrl) that the arduino will accept
Command cmdSys[] = {
    Command("idSketch",  &sendSketchId),
    Command("idBuild",   &sendSketchBuild),
    Command("listCmd",   &sendListCmd,      "s,s",      "*,short|full"),  // eg: :cmdSys,short  or  :,full
    Command("listPin",   &sendListPin),
    Command("pinMode",   &cmdPinMode,       "i,s",      "0-30,o|i|ip"),
    Command("pinRead",   &cmdPinRead,       "i,s",      "0-30,d|a"),
    Command("pinWrite",  &cmdPinWrite,      "i,s,i",    "0-30,d|a,*")
};
CommandList cmdLSys("cmdSys", "AT+", SIZE_OF_TAB(cmdSys), cmdSys );


/*---------------------------------------------------------------*/
/*                setup and loop function                        */
/*---------------------------------------------------------------*/

void setup()
{
    pinMode(pinLed, OUTPUT);

    Serial.begin(115200);

    serList.addCmdList(cmdLUser);
    serList.addCmdList(cmdLSys);

    // I fill info on sketch
    sketchInfo.setFileDateTime(F(__FILE__), F(__DATE__), F(__TIME__));
    sketchInfo.addListPin(F("13:led"));

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

