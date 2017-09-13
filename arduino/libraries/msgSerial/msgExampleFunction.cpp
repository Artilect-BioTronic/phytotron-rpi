#include "Arduino.h"

#include "msgExampleFunction.h"

#include "msgSerial.h"

float globVarConsignHumidity;

static const uint8_t PIN_LED13=13;

/*---------------------------------------------------------------*/
/*           function to test library                            */
/*---------------------------------------------------------------*/

// the lib tests arguments
// that callback  does nothing, it just sends back args
int sendBackArg_s(const CommandList& aCL, Command& aCmd, const String& aInput)
{
    ParsedCommand parsedCmd(aCL, aCmd, aInput);

    // verify that msg with arguments is OK with format and limit
    if (parsedCmd.verifyFormatMsg(aCmd, aInput) != ParsedCommand::NO_ERROR)
        return aCL.returnKO(aCmd, parsedCmd);

    String sValue = parsedCmd.getValueStr(1);

    if (parsedCmd.hasError())
        return aCL.returnKO(aCmd, parsedCmd);


    // I send back OK msg
    aCL.msgOK(aInput, sValue);

    return 0;
}

// the lib tests arguments
// that callback  does nothing, it just sends back args
int sendBackArg_is(const CommandList& aCL, Command& aCmd, const String& aInput) {
    ParsedCommand parsedCmd(aCL, aCmd, aInput);

    // verify that msg with arguments is OK with format and limit
    if (parsedCmd.verifyFormatMsg(aCmd, aInput) != ParsedCommand::NO_ERROR)
        return aCL.returnKO(aCmd, parsedCmd);

    // get values
    int    iValue1 = parsedCmd.getValueInt(1);
    String sValue2 = parsedCmd.getValueStr(2);

    // we check that parsedCmd has not detected any error
    if (parsedCmd.hasError())
        return aCL.returnKO(aCmd, parsedCmd);


    // ok, so we dont make command, this a test, we send back arg

    // I send back OK msg
    aCL.msgOK(aInput, String("arg 1,2:") + iValue1 +","+ sValue2 );

    return 0;
}

// the lib tests arguments
// that callback  does nothing, it just sends back args
int sendBackArg_fs(const CommandList& aCL, Command& aCmd, const String& aInput) {
    ParsedCommand parsedCmd(aCL, aCmd, aInput);

    // verify that msg with arguments is OK with format and limit
    if (parsedCmd.verifyFormatMsg(aCmd, aInput) != ParsedCommand::NO_ERROR)
        return aCL.returnKO(aCmd, parsedCmd);

    // get values
    float  fValue1 = parsedCmd.getValueFloat(1);
    String sValue2 = parsedCmd.getValueStr(2);

    // we check that parsedCmd has not detected any error
    if (parsedCmd.hasError())
        return aCL.returnKO(aCmd, parsedCmd);


    // ok, so we dont make command, this a test, we send back arg

    // I send back OK msg
    aCL.msgOK(aInput, String("arg 1:") + fValue1 +","+ sValue2 );

    return 0;
}

// the lib tests arguments
// that callback  does nothing, it just sends back args
int sendBackArg_ss(const CommandList& aCL, Command& aCmd, const String& aInput) {
    ParsedCommand parsedCmd(aCL, aCmd, aInput);

    // verify that msg with arguments is OK with format and limit
    if (parsedCmd.verifyFormatMsg(aCmd, aInput) != ParsedCommand::NO_ERROR)
        return aCL.returnKO(aCmd, parsedCmd);

    // get values
    String sValue1 = parsedCmd.getValueStr(1);
    String sValue2 = parsedCmd.getValueStr(2);

    // we check that parsedCmd has not detected any error
    if (parsedCmd.hasError())
        return aCL.returnKO(aCmd, parsedCmd);


    // ok, so we dont make command, this a test, we send back arg

    // I send back OK msg
    aCL.msgOK(aInput, String("arg 1,2:") + sValue1 +","+ sValue2 );

    return 0;
}

/*---------------------------------------------------------------*/
/*       common function to try message command library          */
/*---------------------------------------------------------------*/

// switch the led On or Off
int switchLed13(const CommandList& aCL, Command& aCmd, const String& aInput)
{
    ParsedCommand parsedCmd(aCL, aCmd, aInput);

    // verify that msg with arguments is OK with format and limit
    if (parsedCmd.verifyFormatMsg(aCmd, aInput) != ParsedCommand::NO_ERROR)
        return aCL.returnKO(aCmd, parsedCmd);

    String sValue = parsedCmd.getValueStr(1);

    if (parsedCmd.hasError())
        return aCL.returnKO(aCmd, parsedCmd);

    // converts ON / OFF  to  1 / 0
    if (sValue.equals("ON"))
        digitalWrite(PIN_LED13, HIGH);
    else if (sValue.equals("OFF"))
        digitalWrite(PIN_LED13, LOW);
    else
        return aCL.returnKO(aCmd, parsedCmd);

    // I send back OK msg
    aCL.msgOK(aInput, sValue);
    aCL.msgPrint(aCL.getCommand(aInput) + F("/state:") + sValue);

    return 0;
}


int blinkTime=1000;   // used by blinkTime function

// change the blink period of the led
//   led will blink thanks to blinkLed called in loop
int ledBlinkTime(const CommandList& aCL, Command& aCmd, const String& aInput)
{
    ParsedCommand parsedCmd(aCL, aCmd, aInput);

    // verify that msg with arguments is OK with format and limit
    if (parsedCmd.verifyFormatMsg(aCmd, aInput) != ParsedCommand::NO_ERROR)
        return aCL.returnKO(aCmd, parsedCmd);

    // get values
    int iValue = parsedCmd.getValueInt(1);

    // we check that parsedCmd has not detected any error
    if (parsedCmd.hasError())
        return aCL.returnKO(aCmd, parsedCmd);

    blinkTime = iValue;

    // I send back OK msg
    aCL.msgOK(aInput, String(iValue));

    return 0;
}

// that function is called in the loop
// it blinks LED at speed blinkTime (global variable)
void blinkLed() {
  static long lastChange=0;
  static int  blinkState=0;

  if (millis() - lastChange > blinkTime)   {
    lastChange = millis();
    blinkState = ! blinkState;
    digitalWrite(PIN_LED13, blinkState);
  }
}


int sendMultiValue(const CommandList& aCL, Command& aCmd, const String& dumb)
{
    int sensorVal = fakeValue(100, 199);
    aCL.msgOK(dumb, String("/mulTest:") +sensorVal +","+fakeValue(200,299) +","+fakeValue(300,399));
    return 0;
}

int fakeValue(int deb, int fin)
{
    if (deb >= fin)
        return millis() % 1024;

    // fake sensor value
    return millis() % (fin - deb) + deb;
}

int sendFakeDate(const CommandList& aCL, Command& aCmd, const String& dumb)
{
    aCL.msgOK(dumb, String("admin/piClock:") +fakeDate(1));
    return 0;
}

String fakeDate(int iFormat)   {
    if (iFormat<1 || iFormat > 1)
        return String("2001-01-03T12:34:55");
    return String("2017-05-08T") +"09:" + (millis() %60+10) +":"+ (millis() %60+10);
}

// "f",  "0-100"
// temporairement "s",  ""
int updateGlobalVar(const CommandList& aCL, Command& aCmd, const String& aInput)
{
    ParsedCommand parsedCmd(aCL, aCmd, aInput);

    // verify that msg with arguments is OK with format and limit
    if (parsedCmd.verifyFormatMsg(aCmd, aInput) != ParsedCommand::NO_ERROR)
        return aCL.returnKO(aCmd, parsedCmd);

    // get values
    String sValue = parsedCmd.getValueStr(1);

    // we check that parsedCmd has not detected any error
    if (parsedCmd.hasError())
        return aCL.returnKO(aCmd, parsedCmd);


    float fValue = sValue.toFloat();
    // toInt will return 0, if it is not an int
    if ( fabs(fValue) < 0.0001 && ( ! sValue.startsWith("0")) )   {
      aCL.msgKO(aInput, F(" value must be float"));
      return 2;
    }
    else if (fValue < 0 || fValue > 100)   {
      aCL.msgKO(aInput, F(" cmd: value must be in [0-100]"));
      return 3;
    }

    globVarConsignHumidity = fValue;

    // I send back OK msg
    aCL.msgOK(aInput, String(fValue));

    return 0;
}

/*---------------------------------------------------------------*/
/*           functions to make system manipulation               */
/*---------------------------------------------------------------*/

SketchInfo sketchInfo;

// it extracts basename from  aFullFilename
//   because  __FILE__  contains the full path. we strip it
// to be called like this:  setFile(F(__FILE__))
void SketchInfo::setFile(const String& aFullFilename)   {
    int lastSlash = aFullFilename.lastIndexOf('/') +1;  // '/' is not for windows !
    // if there is no '/',  lastIndexOf returns -1, so lastSlash=0
    file_ = aFullFilename.substring(lastSlash);
}

void SketchInfo::setFileDateTime(const String &aFullFilename, const String &aDate,const String &aTime)
{
    setFile(aFullFilename);
    setDate(aDate);
    setTime(aTime);
}

// send an identifiant of arduino sketch: it sends the name of the file !
// you have to define the var  in your sketch.ino
//   with   setFile(F(__FILE__))
int sendSketchId(const CommandList& aCL, Command& aCmd, const String& aInput)
{
    String msg2py= "idSketch:" + sketchInfo.getFile() ;
    aCL.msgPrint(msg2py);
    return 0;
}


// send an identifiant for the version of the sketch
//   we use the __DATE__ and __TIME__ when it was built
// you have to define those  var  in your sketch.ino
//   with   sketchInfo.setFileDateTime(F(__FILE__), F(__DATE__), F(__TIME__))
int sendSketchBuild(const CommandList& aCL, Command& aCmd, const String& dumb)
{
    String msg2py= "idBuild:" + sketchInfo.getDate() +','+ sketchInfo.getTime() ;
    aCL.msgPrint(msg2py);
    return 0;
}

// send the list of cmd available
int sendListCmd(const CommandList& aCL, Command& aCmd, const String& aInput)
{
    ParsedCommand parsedCmd(aCL, aCmd, aInput);

    // verify that msg with arguments is OK with format and limit
    if (parsedCmd.verifyFormatMsg(aCmd, aInput) != ParsedCommand::NO_ERROR)
        return aCL.returnKO(aCmd, parsedCmd);

    // get values
    String nameCL = parsedCmd.getValueStr(1);
    String sMode  = parsedCmd.getValueStr(2);

    // we check that parsedCmd has not detected any error
    if (parsedCmd.hasError())
        return aCL.returnKO(aCmd, parsedCmd);

    int cr = aCL.getSerialListener()->displayListCmd(nameCL, sMode);

    if (cr == 0)
        aCL.msgOK(aInput, String(cr));
    else
        aCL.msgKO(aInput, String(cr));
    return cr;
}

// send the list of Pin definition
int sendListPin(const CommandList& aCL, Command& aCmd, const String& dumb)
{
    aCL.msgPrint(sketchInfo.getListPin());
    aCL.msgOK(dumb, "");
    return 0;
}


int cmdPinMode(const CommandList& aCL, Command& aCmd, const String& aInput) {
    ParsedCommand parsedCmd(aCL, aCmd, aInput);

    // verify that msg with arguments is OK with format and limit
    if (parsedCmd.verifyFormatMsg(aCmd, aInput) != ParsedCommand::NO_ERROR)
        return aCL.returnKO(aCmd, parsedCmd);

    // get values
    int    iValue1 = parsedCmd.getValueInt(1);
    String sValue2 = parsedCmd.getValueStr(2);

    // we check that parsedCmd has not detected any error
    if (parsedCmd.hasError())
        return aCL.returnKO(aCmd, parsedCmd);


    // ok, so we can make command
    if (sValue2 == "i")
        pinMode(iValue1, INPUT);
    else if (sValue2 == "o")
        pinMode(iValue1, OUTPUT);
    else if (sValue2 == "ip")
        pinMode(iValue1, INPUT_PULLUP);
    else   {
        aCL.msgKO(aInput, sValue2);
        return 0;
    }

    // I send back OK msg
    aCL.msgOK(aInput, "" );

    return 0;
}

// cmd a analogRead or digitalRead
// value fmt: pin (0-20), 1=digitalRead/2=analogRead
int cmdPinRead(const CommandList& aCL, Command& aCmd, const String& aInput) {
    ParsedCommand parsedCmd(aCL, aCmd, aInput);

    // verify that msg with arguments is OK with format and limit
    if (parsedCmd.verifyFormatMsg(aCmd, aInput) != ParsedCommand::NO_ERROR)
        return aCL.returnKO(aCmd, parsedCmd);

    // get values
    int    iValue1 = parsedCmd.getValueInt(1);
    String sValue2 = parsedCmd.getValueStr(2);

    // we check that parsedCmd has not detected any error
    if (parsedCmd.hasError())
        return aCL.returnKO(aCmd, parsedCmd);

    // ok, so we can make command
    int iValue=0;
    if (sValue2 == "d")
      iValue = digitalRead(iValue1);
    else   // iValue2 == a
      iValue = analogRead(iValue1);

    // I send back OK msg
    aCL.msgOK(aInput, String(iValue));

    return 0;
}

// cmd a digitalWrite or analogWrite (PWM)
// value fmt: pin (0-20), 1=digitalRead/2=analogRead, value
// fmt and lim:  "i,s,i",    "0-30,d|a,*"
int cmdPinWrite(const CommandList& aCL, Command &aCmd, const String& aInput) {
    ParsedCommand parsedCmd(aCL, aCmd, aInput);

    // verify that msg with arguments is OK with format and limit
    if (parsedCmd.verifyFormatMsg(aCmd, aInput) != ParsedCommand::NO_ERROR)
        return aCL.returnKO(aCmd, parsedCmd);

    // get values
    int    iValue1 = parsedCmd.getValueInt(1);
    String sValue2 = parsedCmd.getValueStr(2);
    int    iValue3 = parsedCmd.getValueInt(3);

    // we check that parsedCmd has not detected any error
    if (parsedCmd.hasError())
        return aCL.returnKO(aCmd, parsedCmd);

    // ok, so we can make command
    if (sValue2 == "d")
      digitalWrite(iValue1,iValue3);
    else   // iValue2 == a
      analogWrite(iValue1,iValue3);

    // I send back OK msg
    aCL.msgOK(aInput, "");

    return 0;
}


/*---------------------------------------------------------------*/
/*                  list of user function                        */
/*---------------------------------------------------------------*/

int sendFakeVal(const CommandList &aCL, Command& aCmd, const String& dumb)
{
    int sensorVal = getSensorValue();
    aCL.msgPrint(aCL.getCommand(dumb) +"/temp:"+ sensorVal);
    aCL.msgOK(dumb, String(sensorVal));
    return 0;
}

int sendMessageStatus(const CommandList& aCL, Command &aCmd, const String& dumb)
{
    int sensorVal = getSensorValue();
    aCL.msgPrint(aCL.getCommand(dumb) +"/temp:"+ sensorVal);
    aCL.msgOK(dumb, String(sensorVal));
    return 0;
}

int getSensorValue()
{
  // fake sensor value
  return millis() % 1024;
}


/*---------------------------------------------------------------*/
/*                                          */
/*---------------------------------------------------------------*/

