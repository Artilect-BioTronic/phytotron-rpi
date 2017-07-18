#include "Arduino.h"

#define LOG_DEBUG(str)   Serial.println(str)
#define LOG_ERROR(str)   msgSError(str)

#include "msgSerial.h"

#include "SPI.h"
#include "SdFat.h"
//#include "msg2SDCard.h"


#define PIN_LED13 13

//float globVarConsignHumidity=0.;


// list of available commands (user) that the arduino will accept
int switchLed1(const CommandList& aCL, const String& dumb);

SerialListener serList(Serial);

Command cmdUser[] = {
    Command("SendValue",     &sendMessageStatus),
    Command("S",             &sendMessageStatus),
    Command("MultiValue",    &sendMultiValue),
    Command("lit1/switch",   &switchLed1),
    Command("sl13",          &switchLed13)
};
CommandList cmdLUser("cmdUser", "CM+", SIZE_OF_TAB(cmdUser), cmdUser );

// list of available commandes (system ctrl) that the arduino will accept
// example:  int sendSketchId(const String& dumb);

// list of available commands (user) that the arduino will accept

// list of available commands (system ctrl) that the arduino will accept
Command cmdSys[] = {
    Command("idSketch",  &sendSketchId),
    Command("idBuild",   &sendSketchBuild),
    Command("listCmdAT", &sendListCmd),
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
    pinMode(PIN_LED13, OUTPUT);
    Serial.begin(9600);
    serList.addCmdList(cmdLUser);
    serList.addCmdList(cmdLSys);
    
    // I fill info on sketch
    sketchInfo.setFileDateTime(F(__FILE__), F(__DATE__), F(__TIME__));
    // I send identification of sketch
    sendSketchId(cmdLSys, "");
    sendSketchBuild(cmdLSys, "");
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
int switchLed1(const CommandList& aCL, const String& sOnOff)
{
    return switchLed13(aCL, sOnOff);
}

