#include "Arduino.h"

#define LOG_DEBUG(str)   Serial.println(str)
#define LOG_ERROR(str)   msgSError(str)

#include "msgSerial.h"

#include "SPI.h"
#include "SdFat.h"
#include "msg2SDCard.h"


#define PIN_CAPTOR  10
#define PIN_LED13 13
stListPin listPin[] = {
  stListPin(PIN_LED13, "LED"),
  stListPin(PIN_CAPTOR, "CAPTOR unknownTest")
};
int listPinSize = sizeof(listPin) / sizeof(stListPin);


// list of available commands (user) that the arduino will accept
int switchLed1(const String& dumb);
int switchLed13(const String& dumb);

SerialListener serList(Serial);

Command cmdUser[] = {
    Command("SendValue",     &sendMessageStatus),
    Command("S",             &sendMessageStatus),
//    Command("srPreOpen",     &srPreOpen),    // :file
//    Command("srClose",       &srClose),   // :
//    Command("srReadln",      &srReadln),  // :
//    Command("srRead",        &srRead),  // : ?
//    Command("srWrite",       &srWrite),
//    Command("srMove",        &srMove),
    Command("sl13",          &switchLed13),
    Command("lit1/switch",   &switchLed1)
};
CommandList cmdLUser("cmdUser", "CM+", SIZE_OF_TAB(cmdUser), cmdUser );

// list of available commandes (system ctrl) that the arduino will accept
// example:  int sendSketchId(const String& dumb);

// list of available commands (user) that the arduino will accept

// list of available commands (system ctrl) that the arduino will accept
Command cmdSys[] = {
    Command("idSketch",  &sendSketchId),
    Command("idBuild",   &sendSketchBuild),
    Command("listCmdAT", &sendListCmdAT),
    Command("listCmdDO", &sendListCmdDO),
    Command("listPin",   &sendListPin),
    Command("pinMode",   &cmdPinMode),
    Command("pinRead",   &cmdPinRead),
    Command("pinWrite",  &cmdPinWrite)
};
CommandList cmdLSys("cmdSys", "AT+", SIZE_OF_TAB(cmdSys), cmdSys );


/*---------------------------------------------------------------*/
/*                setup and loop function                        */
/*---------------------------------------------------------------*/

void setup()
{
    pinMode(PIN_LED, OUTPUT);
    Serial.begin(9600);
    serList.addCmdList(cmdLUser);
    serList.addCmdList(cmdLSys);
    
    // I fill info on sketch
    sketchInfo.setFileDateTime(F(__FILE__), F(__DATE__), F(__TIME__));
    // I send identification of sketch
    sendSketchId("");
    sendSketchBuild("");
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
int switchLed1(const String& sOnOff)
{
    return switchLed13(sOnOff);
}

// switch the led On or Off
int switchLed13(const String& sOnOff)
{
    // sCmdAndBlinkTime contains cmd and value with this format cmd:value
    // value must exist
    int ind = sOnOff.indexOf(":");
    if (ind < 0)   {
      msgSError(F("switchLed cmd needs 1 value"));
      msgSPrint(F("switchLed/KO"));
      return 1;
    }

    // we get value part
    String sValue = sOnOff.substring(ind+1);
    // value must be  ON  or  OFF
    if ( ( ! sValue.equals("ON")) && ( ! sValue.equals("OFF")) )   {
      msgSError(F("switchLed: value must be ON or OFF"));
      msgSPrint(F("switchLed/KO"));
      return 2;
    }

    int iValue=0;
    // converts ON / OFF  to  1 / 0
    if (sValue.equals("ON"))
      iValue = 1;
    else if (sValue.equals("OFF"))
      iValue = 0;
    else
        msgSPrint(F("switchLed/KO"));


    digitalWrite(PIN_LED13, iValue);

    // I send back OK msg
    msgSPrint(String(F("switchLed/OK:")) + sValue);
    // I send back state msg
    msgSPrint(String(F("state:")) + sValue);

    return 0;
}

