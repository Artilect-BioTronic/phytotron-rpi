#include "Arduino.h"

#include "TempHumMsg.h"

#include "msgSerial.h"


SerialListener serList(Serial);

Command cmdUserPhy[] = {
    Command("SV",              &sendFakeVal),
    Command("change/NameVal",  &changeNameVal),
    Command("sl13",            &switchLed13)
};
CommandList cmdLUserPhy("cmdUser", "CM+", SIZE_OF_TAB(cmdUserPhy), cmdUserPhy );

// list of available commandes (system ctrl) that the arduino will accept
// example:  int sendSketchId(const String& dumb);

// list of available commands (user) that the arduino will accept

// list of available commands (system ctrl) that the arduino will accept
Command cmdSysPhy[] = {
    Command("idSketch",         &sendSketchId),
    Command("idBuild",          &sendSketchBuild),
    Command("SV",               &sendMessageStatus),
    Command("MultiValue",       &sendMultiValue),
    Command("sendDate",         &sendDate),
    Command("csgn/humidityIn/cmd",    &updateHumCsgn)
};
CommandList cmdLSysPhy("cmdSys", "AT+", SIZE_OF_TAB(cmdSysPhy), cmdSysPhy );

/*---------------------------------------------------------------*/
/*                                          */
/*---------------------------------------------------------------*/

// switch the led On or Off
int changeNameVal(const String& astr)
{
    // switchLed13 contains cmd and value with this format cmd:value
    // value must exist
    int ind = astr.indexOf(":");
    if (ind < 0)   {
      msgSError(F("switchLed cmd needs 1 value"));
      msgSPrint(F("switchLed/KO"));
      return 1;
    }

    // we get value part
    String sValue = astr.substring(ind+1);
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
