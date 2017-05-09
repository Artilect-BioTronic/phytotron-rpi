#include "Arduino.h"

#include "msgExampleFunction.h"

#include "msgSerial.h"

float globVarConsignHumidity;

/*---------------------------------------------------------------*/
/*                                          */
/*---------------------------------------------------------------*/

// switch the led On or Off
int switchLed13(const String& sOnOff)
{
    // switchLed13 contains cmd and value with this format cmd:value
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


int sendMultiValue(const String& dumb)
{
    int sensorVal = fakeValue(100, 199);
    msgSPrint(String("mulTest:") +sensorVal +","+fakeValue(200,299) +","+fakeValue(300,399));
    return 0;
}

int fakeValue(int deb, int fin)
{
    if (deb >= fin)
        return millis() % 1024;

    // fake sensor value
    return millis() % (fin - deb) + deb;
}

int sendDate(const String& dumb)
{
    msgSPrint(String("admin/piClock:") +fakeDate(1));
    return 0;
}

String fakeDate(int iFormat)   {
    if (iFormat<1 || iFormat > 1)
        return String("2001-01-03T12:34:55");
    return String("2017-05-08T") +"09:" + (millis() %60+10) +":"+ (millis() %60+10);
}

int updateHumCsgn(const String& aStr)
{
    // updateHumCsgn contains cmd and value with this format cmd:value
    // value must exist
    int ind = aStr.indexOf(":");
    if (ind < 0)   {
      msgSError(getCommand(aStr) + F(" cmd needs 1 value"));
      msgSPrint(getCommand(aStr) + "/KO" +1);
      return 1;
    }

    // we get value part
    String sValue = aStr.substring(ind+1);

    int fValue = sValue.toFloat();
    // toInt will return 0, if it is not an int
    if ( fabs(fValue) < 0.0001 && ( ! sValue.startsWith("0")) )   {
      msgSError(getCommand(aStr) + F(" cmd: value must be float"));
      return 2;
    }
    else if (fValue < 0 || fValue > 100)   {
      msgSError(getCommand(aStr) + F(" cmd: value must be in [0-100]"));
      return 3;
    }

    globVarConsignHumidity = fValue;

    // I send back OK msg
    msgSPrint(getCommand(aStr) + "/OK:" +fValue);
    // I send back state msg
    //msgSPrint(String(F("state:")) + sValue);

    return 0;
}
