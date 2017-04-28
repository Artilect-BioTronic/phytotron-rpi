#include "Arduino.h"

#define LOG_DEBUG(str)   Serial.println(str)
#define LOG_ERROR(str)   Serial.println(str)

#include "msgSerial.h"

DEFINE_FILE_DATE_TIME

#define PIN_CAPTOR  10
#define PIN_LED 13
stListPin listPin[] = {
  stListPin(PIN_LED, "LED"),
  stListPin(PIN_CAPTOR, "CAPTOR unknownTest")
};
int listPinSize = sizeof(listPin) / sizeof(stListPin);


// list of available commands (user) that the arduino will accept
int switchLed1(const String& dumb);


CommandC cmdcos[] = {
  CommandC("SendValue",     &sendMessageStatus),
  CommandC("S",             &sendMessageStatus),
  CommandC("ledBlink",      &ledBlinkTime),
  CommandC("lit1/switch",   &switchLed1)
};
//CommandC cmc5("tt",   &switchLed1);

Command cmdTmp[] = {
  Command("SendValue",     &sendMessageStatus),
  Command("S",             &sendMessageStatus),
  Command("sl",      &switchLed1),
  Command("lit1/switch",   &switchLed1)
};
CommandList cmdl("CMDl", "CM+", SIZE_OF_TAB_AND_TAB(cmdTmp) );
SerialListener serList(Serial);

// list of available commandes (system ctrl) that the arduino will accept
// example:  int sendSketchId(const String& dumb);

// list of available commands (user) that the arduino will accept
Command cmdos[] = {
  Command("SendValue",     &sendMessageStatus),
  Command("S",             &sendMessageStatus),
  Command("sl",            &switchLed1),
  Command("lit1/switch",   &switchLed1)
};
int cmdosSize = sizeof(cmdos) / sizeof(Command);

// list of available commands (system ctrl) that the arduino will accept
Command cmds[] = {
  Command("idSketch",  &sendSketchId),
  Command("idBuild",   &sendSketchBuild),
  Command("listCmdAT", &sendListCmdAT),
  Command("listCmdDO", &sendListCmdDO),
  Command("listPin",   &sendListPin),
  Command("pinMode",   &cmdPinMode),
  Command("pinRead",   &cmdPinRead),
  Command("pinWrite",  &cmdPinWrite)
};
int cmdsSize = sizeof(cmds) / sizeof(Command);


/*---------------------------------------------------------------*/
/*                setup and loop function                        */
/*---------------------------------------------------------------*/

void setup()
{
    pinMode(PIN_LED, OUTPUT);
    Serial.begin(9600);
    serList.addCmdList(cmdl);
    
    inputMessage.reserve(200);
    
    // I send identification of sketch
//    sendSketchId("");
//    sendSketchBuild("");
    
}
 
void loop()
{
//  checkMessageReceived();
  serList.checkMessageReceived();

//  static bool etatLed=true;
//  etatLed = ! etatLed;
//  digitalWrite(13,etatLed);

  // I slow down Arduino
  delay(10);
}


/*---------------------------------------------------------------*/
/*                  list of user function                        */
/*---------------------------------------------------------------*/

// switch the led On or Off
int switchLed1(const String& sOnOff)
{
    // sCmdAndBlinkTime contains cmd and value with this format cmd:value
    // value must exist
    int ind = sOnOff.indexOf(":");
    if (ind < 0)   {
      LOG_ERROR(F("switchLed cmd needs 1 value"));
      String msg2py = msg2pyStart + "switchLed/KO" + msg2pyEnd;
      Serial.print(msg2py);
      return 1;
    }
    
    // we get value part
    String sValue = sOnOff.substring(ind+1);
    // value must be  ON  or  OFF
    if ( ( ! sValue.equals("ON")) && ( ! sValue.equals("OFF")) )   {
      LOG_ERROR(F("switchLed: value must be ON or OFF"));
      String msg2py = msg2pyStart + F("switchLed/KO") + msg2pyEnd;
      Serial.print(msg2py);
      return 2;
    }

    int iValue=0;
    // converts ON / OFF  to  1 / 0
    if (sValue.equals("ON"))
      iValue = 1;
    else if (sValue.equals("OFF"))
      iValue = 0;
    else
      Serial.println ("jamais de la vie");
   
    digitalWrite(PIN_LED, iValue);
    
    // I send back OK msg
    String msg2py = msg2mqttStart + F("switchLed/OK:") + sValue + msg2pyEnd;
    Serial.print(msg2py);
    // I send back state msg
    msg2py = msg2mqttStart + F("etat:") + sValue + msg2pyEnd;
    Serial.print(msg2py);
    
    return 0;
}

// switch the led On or Off
int switchLed2(const String& sOnOff)
{
    // sCmdAndBlinkTime contains cmd and value with this format cmd:value
    // value must exist
    int ind = sOnOff.indexOf(":");
    if (ind < 0)   {
      LOG_ERROR(F("switchLed cmd needs 1 value"));
      String msg2py = msg2pyStart + "switchLed/KO" + msg2pyEnd;
      Serial.print(msg2py);
      return 1;
    }
    
    // we get value part
    String sValue = sOnOff.substring(ind+1);
    // value must be  ON  or  OFF
    if ( ( ! sValue.equals("ON")) && ( ! sValue.equals("OFF")) )   {
      LOG_ERROR(F("switchLed: value must be ON or OFF"));
      String msg2py = msg2pyStart + F("switchLed/KO") + msg2pyEnd;
      Serial.print(msg2py);
      return 2;
    }

    int iValue=0;
    // converts ON / OFF  to  1 / 0
    if (sValue.equals("ON"))
      iValue = 1;
    else if (sValue.equals("OFF"))
      iValue = 0;
    else
      Serial.println ("jamais de la vie");
   
    digitalWrite(PIN_LED, iValue);
    
    // I send back OK msg
    String msg2py = msg2mqttStart + F("switchLed/OK:") + sValue + msg2pyEnd;
    Serial.print(msg2py);
    // I send back state msg
    msg2py = msg2mqttStart + F("etat:") + sValue + msg2pyEnd;
    Serial.print(msg2py);
    
    return 0;
}


