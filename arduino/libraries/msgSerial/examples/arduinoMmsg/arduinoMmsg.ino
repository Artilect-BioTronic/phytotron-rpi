#include "Arduino.h"

#define LOG_DEBUG(str)   Serial.println(str)
#define LOG_ERROR(str)   msgSError(str)

#include "msgSerial.h"

#include "SPI.h"
#include "SdFat.h"
#include "msg2SDCard.h"


#define PIN_LED13 13

float globVarConsignHumidity=0.;


// list of available commands (user) that the arduino will accept
int switchLed1(const String& dumb);
int switchLed13(const String& dumb);
int sendMultiValue(const String& dumb);
int sendDate(const String& dumb);
int updateHumCsgn(const String& dumb);

int fakeValue(int deb=0, int fin=1023);
String fakeDate(int iFormat=1);

SerialListener serList(Serial);

Command cmdUser[] = {
    Command("SendValue",     &sendMessageStatus),
    Command("S",             &sendMessageStatus),
    Command("MultiValue",    &sendMultiValue),
    Command("sendDate",      &sendDate),
    Command("csgn/humidityIn/cmd",    &updateHumCsgn),
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
    pinMode(PIN_LED13, OUTPUT);
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
