#include "Arduino.h"

#include "msgSerial.h"

#define LOG_DEBUG(str)   Serial.println(str)
#define LOG_ERROR(str)   SERIAL_MSG.println(str)

SketchInfo sketchInfo;

String inputMessage = "";
boolean inputMessageReceived = false;
String msg2pyStart = "AT+";     // "2py;";
String msg2mqttStart = "CM+";     // "2mq;";
String msg2pyEnd = "\n";
String prefAT = "AT+";
String prefDO = "DO+";

// listPin and listPinSize are defined in main sketch.ino
// see ex of definition in msgFromMQTT.h
extern stListPin listPin[];
extern int listPinSize;
// ex of definition
/*
// #define PIN_CAPTOR  10
// #define PIN_LED 2
stListPin listPin[] = {
  stListPin(PIN_LED, "LED"),
  stListPin(PIN_CAPTOR, "CAPTOR unknown")
};
int listPinSize = sizeof(listPin) / sizeof(stListPin);
*/


// functions to send back messages

size_t msgSPrint(const String& aMsg)   {
    String msg2py = msg2mqttStart + aMsg + msg2pyEnd;
    return SERIAL_MSG.print(msg2py);
}

size_t msgSPrintln(const String& aMsg)   {
    return msgSPrint(aMsg + "\n");
}

size_t msgSError(const String& aMsg)   {
    return SERIAL_MSG.println(aMsg);
}

size_t msgSPrint2(const String& aMsg)   {
    String msg2py = msg2pyStart + aMsg + msg2pyEnd;
    return SERIAL_MSG.print(msg2py);
}


uint8_t CommandC::nbObjects = 0;
CommandC* CommandC::arrayObjects[nbObjectsMax];

int CommandC::addCmdInArray(CommandC* pCmd) {
    if (nbObjects >= nbObjectsMax) {
        LOG_ERROR(F("too many CommandC objects created"));
        return -1;
    }
    arrayObjects[nbObjects] = pCmd;
    nbObjects++;
    return 0;
}

CommandC* CommandC::getObject(uint8_t i) {
    if ( (i > nbObjectsMax) || (i <0) )
        return NULL;

    return arrayObjects[i];
}


int blinkTime=1000;   // used by blinkTime function

// example of serialEvent  you have to put in your sketch.ino
//   note that it empties serial buffer immediately
//   note that it does not test  memory allocation  for inputMessage
//   in case the msg never ends, because there is no  \n end of line
// note that  serialEvent() is useless, you can call  checkMessageReceived
//   directly in loop.
/*
void serialEvent() 
{
  serList.checkMessageReceived();
}
*/

CommandList::CommandList(const String &nameListCmd, const String &prefix,
                         const int nbObjects, const Command arrayCmd[],
                         char cmdSeparator, char argSeparator, char cmdEnd):
    _nameListCmd(nameListCmd), _prefix(prefix), _nbObjects(nbObjects), _arrayCmd(arrayCmd),
    _cmdSeparator(cmdSeparator), _argSeparator(argSeparator), _cmdEnd(cmdEnd)
{}

SerialListener::SerialListener(Stream &aStream): _stream(aStream), _nbObjects(0),
    _inputMessage(""), _inputMessageReceived(false), _timeLastIncomingChar(0), _timeOut(100),
    _cmdEnds("")
{}

bool SerialListener::addCmdList(CommandList &cmdL)   {
    if ( _nbObjects < _nbObjectsMax )   {
        _arrayObjects[_nbObjects] = &cmdL;
        _nbObjects++;
        _cmdEnds += cmdL.getCmdEnd();
    }
}

// returns true if cmd has been found; even if the cmd fails
bool CommandList::checkMessageReceived(String aInputMessage)
{
    // if it is a cmd for user
    if (aInputMessage.startsWith(_prefix))
    {
        LOG_DEBUG(String(F("Message is a ")) + _nameListCmd + F(" cmd, I am parsing..."));

        String nudeMessage = aInputMessage.substring(_prefix.length());
        // we get rid of end character, if it exists
        if ( nudeMessage.charAt(nudeMessage.length()-1) == _cmdEnd )
            nudeMessage = nudeMessage.substring(0, nudeMessage.length()-1);

        // we extract the cmd name
        String command = "";
        int iCmdSeparator = nudeMessage.indexOf(_cmdSeparator);
        if (iCmdSeparator > 0)
            command = nudeMessage.substring(0, iCmdSeparator);
        else   {
            // if there is no arg => no separator,  indexOf returns -1
            // so cmd goes from prefix to end of string
            command = nudeMessage;
        }

        // We search the command among the list
        bool cmdNotFound = true;
        for (int i=0; i<_nbObjects && cmdNotFound; i++)
        {
            if (command == _arrayCmd[i].cmdName)
            {
                cmdNotFound = false;
                int cr = _arrayCmd[i].cmdFunction(nudeMessage);
            }
        }
        if (cmdNotFound)  {
            LOG_ERROR(_nameListCmd + F(" cmd not recognized ! :") + command);
            return false;
        }
        return true;
    }
    else
        return false;
}

void SerialListener::copyBuffer()
{
    // we block msg transfer from _stream buffer to inputMessage, that
    //   means that a 2nd message will stay in buffer, until
    //   inputMessage is analyzed
    while( _stream.available() && ( ! _inputMessageReceived )  )
    {
        char inChar = (char)_stream.read();

        // is inChar among the list of char that ends commands
        if( _cmdEnds.indexOf(inChar) >= 0 )   {
            _inputMessage += inChar;   // "possible end character" is copied
            _inputMessageReceived = true;
        }
        else   {
            _inputMessage += inChar;
            _timeLastIncomingChar = millis();
        }
    }

    // we check there is no blocked stuff in buffer
    // if some stuff is in buffer, and if no more char is coming, we state
    //   the msg might be complete
    if ( (_inputMessage.length()>0) && (millis() - _timeLastIncomingChar > _timeOut) )  {
        if ( _cmdEnds.indexOf(_inputMessage.charAt(_inputMessage.length()-1)) >= 0 )
            // _inputMessage is terminated by a  end character, so the message has been checked
            //   and failed, thus we clean buffer _inputMessage
            _inputMessage = "";
        else
            // we set _inputMessageReceived = true, thus the input msg will be checked (before cleaning)
            _inputMessageReceived = true;
    }
}

// check if a message has been received and analyze it
// in your main sketch.ino, you have to update global var inputMessageReceived and inputMessage
// checkMessageReceived calls the function corresponding to  cmds/cmdos array
void SerialListener::checkMessageReceived()
{
    // copy available data in stream buffer
    copyBuffer();

    if(_inputMessageReceived)
    {
        bool isMsgRecognized=false;
        for (int i=0; i<_nbObjects && (!isMsgRecognized); i++)
                isMsgRecognized |= _arrayObjects[i]->checkMessageReceived(_inputMessage);

        if ( isMsgRecognized )
            _inputMessage = "";
        else   {
            LOG_DEBUG(F("msg is not a recognized cmd:"));
            LOG_DEBUG(_inputMessage);
        }

        // if Msg is not Recognized, we dont erase it, maybe it is incomplete, and
        //   it will be completed later

        // if msg has blocked in buffer for timeOut, we display and empty buffer
        if (millis() - _timeLastIncomingChar > _timeOut)   {
            _inputMessage = "";
        }
    }
    _inputMessageReceived = false;
}


/*---------------------------------------------------------------*/
/*                  list of user function                        */
/*---------------------------------------------------------------*/

int sendMessageStatus(const String& dumb)
{
    int sensorVal = getSensorValue(); 
    msgSPrint(String("temp:") +sensorVal);
    return 0;
}

int getSensorValue()
{
  // fake sensor value
  return millis() % 1024; 
}

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

/*---------------------------------------------------------------*/
/*                  list of system ctrl function                 */
/*---------------------------------------------------------------*/

// send an identiant of arduino sketch: it sends the name of the file !
// you have to define the var  in your sketch.ino
//   with   setFile(F(__FILE__))
int sendSketchId(const String& dumb)
{
    String msg2py= "idSketch:" + sketchInfo.getFile() ;
    msgSPrint2(msg2py);
    return 0;
}


// send an identifiant for the version of the sketch
//   we use the __DATE__ and __TIME__ when it was built
// you have to define those  var  in your sketch.ino
//   with   sketchInfo.setFileDateTime(F(__FILE__), F(__DATE__), F(__TIME__))
int sendSketchBuild(const String& dumb)
{
    String msg2py= "idBuild:" + sketchInfo.getDate() +','+ sketchInfo.getTime() ;
    msgSPrint2(msg2py);
    return 0;
}

// send the list of cmd AT available
int sendListCmdAT(const String& dumb)   // TODO: adapt
{
//    String availCmd = "";
//    if (cmdsSize >= 0)
//       availCmd = cmds[0].cmdName;
//    for (int i=1; i<cmdsSize; i++)
//       availCmd += "," + cmds[i].cmdName;
    
//    String msg2py = msg2pyStart + "listCmdAT" + ":" + availCmd
//                   + msg2pyEnd;
//    Serial.print(msg2py);
    return 0;
}

// send the list of cmd AT available
int sendListCmdDO(const String& dumb)   // TODO: adapt
{
//    String availCmd = "";
//    if (cmdosSize >= 0)
//       availCmd = cmdos[0].cmdName;
//    for (int i=1; i<cmdosSize; i++)
//       availCmd += "," + cmdos[i].cmdName;
    
//    String msg2py = msg2pyStart + "listCmdDO" + ":" + availCmd
//                   + msg2pyEnd;
//    Serial.print(msg2py);
    return 0;
}

// send the list of Pin definition
int sendListPin(const String& dumb)
{
    String availPin = "";
    if (listPinSize >= 0)
       availPin = availPin + " " + listPin[0].numPin +" "+ listPin[0].namePin;
    for (int i=1; i<listPinSize; i++)
       availPin = availPin + ", " + listPin[i].numPin +" "+ listPin[i].namePin;
    
    String msg2py = msg2pyStart + "listPin" + ":" + availPin + msg2pyEnd;
    SERIAL_MSG.print(msg2py);
    return 0;
}

// change the blink period of the led
//   led will blink thanks to blinkLed called in loop
int ledBlinkTime(const String& sCmdAndBlinkTime)
{
    // sCmdAndBlinkTime contains cmd and value with this format cmd:value
    // value must exist
    int ind = sCmdAndBlinkTime.indexOf(":");
    if (ind < 0)   {
      LOG_ERROR(F("ledBlinkTime cmd needs 1 value"));
      String msg2py = msg2pyStart + "ledBlinkTime/KO" + msg2pyEnd;
      SERIAL_MSG.print(msg2py);
      return 1;
    }
    
    // we get value part
    String sValue = sCmdAndBlinkTime.substring(ind+1);
    // value must be an int
    int iValue = sValue.toInt();
    // toInt will return 0, if it is not an int
    if ( (iValue == 0) && ( ! sValue.equals("0")) )   {
      LOG_ERROR(F("ledBlinkTime: value must be 1 integer"));
      String msg2py = msg2pyStart + "ledBlinkTime/KO" + msg2pyEnd;
      SERIAL_MSG.print(msg2py);
      return 2;
    }
    else if (iValue < 0)   {
      LOG_ERROR(F("ledBlinkTime: value must be integer > 0"));
      String msg2py = msg2pyStart + "ledBlinkTime/KO" + msg2pyEnd;
      SERIAL_MSG.print(msg2py);
      return 3;
    }

    blinkTime = iValue;
    
    // I send back OK msg
    String msg2py = msg2pyStart + "ledBlinkTime/OK:" + blinkTime + msg2pyEnd;
    SERIAL_MSG.print(msg2py);
    
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

// switch the led On or Off
int switchLed(const String& sOnOff)
{
    // sCmdAndBlinkTime contains cmd and value with this format cmd:value
    // value must exist
    int ind = sOnOff.indexOf(":");
    if (ind < 0)   {
      LOG_ERROR(F("switchLed cmd needs 1 value"));
      String msg2py = msg2pyStart + "switchLed/KO" + msg2pyEnd;
      SERIAL_MSG.print(msg2py);
      return 1;
    }
    
    // we get value part
    String sValue = sOnOff.substring(ind+1);
    // value must be  ON  or  OFF
    if ( ( ! sValue.equals("ON")) && ( ! sValue.equals("OFF")) )   {
      LOG_ERROR(F("switchLed: value must be ON or OFF"));
      String msg2py = msg2pyStart + F("switchLed/KO") + msg2pyEnd;
      SERIAL_MSG.print(msg2py);
      return 2;
    }

    int iValue=0;
    // converts ON / OFF  to  1 / 0
    if (sValue.equals("ON"))
      iValue = 1;
    else if (sValue.equals("OFF"))
      iValue = 0;
    else
      SERIAL_MSG.println ("jamais de la vie");
   
    digitalWrite(PIN_LED13, iValue);
    
    // I send back OK msg
    String msg2py = msg2mqttStart + F("switchLed/OK:") + sValue + msg2pyEnd;
    SERIAL_MSG.print(msg2py);
    // I send back state msg
    msg2py = msg2mqttStart + F("etat:") + sValue + msg2pyEnd;
    SERIAL_MSG.print(msg2py);
    
    return 0;
}

int cmdPinMode(const String& pin_mode) {
    // pin_mode contains cmd and value with this format cmd:value
    // value must exist
    int ind = pin_mode.indexOf(":");
    if (ind < 0)   {
      LOG_ERROR(F("pinMode cmd needs 2 values"));
      return 1;
    }
    // we get value part
    String sValues = pin_mode.substring(ind+1);
    
    // we separate the 2 values
    ind = pin_mode.indexOf(",");
    if (ind < 0)   {
      LOG_ERROR(F("pinMode cmd needs 2 values"));
      return 2;
    }

    // we get 1st value
    String sValue = sValues.substring(0,ind);
    // value must be an int
    int iValue1 = sValue.toInt();
    // toInt will return 0, if it is not an int
    if ( (iValue1 == 0) && ( ! sValue.equals("0")) )   {
      LOG_ERROR(F("pinMode cmd: value 1 must be integer"));
      return 2;
    }
    else if ((iValue1 < 0) || (20 < iValue1))   {   // note that the value 20 is not accurate
      LOG_ERROR(F("pinMode cmd: value 1 must be compatible with pin num"));
      return 3;
    }

    // we get 2nd value
    sValue = sValues.substring(ind+1);
    // value must be an int
    int iValue2 = sValue.toInt();
    // toInt will return 0, if it is not an int
    if ( (iValue2 == 0) && ( ! sValue.equals("0")) )   {
      LOG_ERROR(F("pinMode cmd: value 2 must be integer"));
      return 4;
    }
    else if ((iValue2 < 0) || (2 < iValue2))   {
      LOG_ERROR(F("pinMode cmd: value 2 must be mode=0/1/2"));
      return 5;
    }

    // ok, so we can make command
    pinMode(iValue1, iValue2);

    // I send back OK msg
    String msg2py = msg2pyStart + "pinMode/OK" + msg2pyEnd;
    SERIAL_MSG.print(msg2py);
    
    return 0;
}

// cmd a analogRead or digitalRead
// value fmt: pin (0-20), 1=digitalRead/2=analogRead
int cmdPinRead(const String& pin_digitAnalog) {
    // pin_mode contains cmd and value with this format cmd:value
    
    // value must exist
    int ind = pin_digitAnalog.indexOf(":");
    if (ind < 0)   {
      LOG_ERROR(F("pinRead cmd needs 2 values"));
      return 1;
    }
    // we get value part
    String sValues = pin_digitAnalog.substring(ind+1);
    
    // we separate the 2 values
    ind = sValues.indexOf(",");
    if (ind < 0)   {
      LOG_ERROR(F("pinRead cmd needs 2 values"));
      return 2;
    }

    // we get 1st value
    String sValue = sValues.substring(0,ind);
    // value must be an int
    int iValue1 = sValue.toInt();
    // toInt will return 0, if it is not an int
    if ( (iValue1 == 0) && ( ! sValue.equals("0")) )   {
      LOG_ERROR(F("pinRead cmd: value 1 must be integer"));
      return 21;
    }
    else if ((iValue1 < 0) || (20 < iValue1))   {   // note that the value 20 is not accurate
      LOG_ERROR(F("pinRead cmd: value 1 must be compatible with pin num"));
      return 3;
    }

    // we get 2nd value
    sValue = sValues.substring(ind+1);
    // value must be an int
    int iValue2 = sValue.toInt();
    // toInt will return 0, if it is not an int
    if ( (iValue2 == 0) && ( ! sValue.equals("0")) )   {
      LOG_ERROR(F("pinRead cmd: value 2 must be integer"));
      return 4;
    }
    else if ((iValue2 < 1) || (2 < iValue2))   {
      LOG_ERROR(F("pinRead cmd: value 2 must be digital/analog=1/2"));
      return 5;
    }

    // ok, so we can make command
    int iValue=0;
    if (iValue2 == 1)
      iValue = digitalRead(iValue1);
    else   // iValue2 == 2
      iValue = analogRead(iValue1);

    // I send back OK msg
    String msg2py = msg2pyStart + "pinRead:" + iValue + msg2pyEnd;
    SERIAL_MSG.print(msg2py);
    
    return 0;
}

// cmd a digitalWrite or analogWrite (PWM) 
// value fmt: pin (0-20), 1=digitalRead/2=analogRead, value
int cmdPinWrite(const String& pin_digitAnalog_val) {
    // pin_digitAnalog_val contains cmd and value with this format cmd:pin,digAn,value
    
    // value must exist
    int ind = pin_digitAnalog_val.indexOf(":");
    if (ind < 0)   {
      LOG_ERROR(F("pinWrite cmd needs 3 values"));
      return 1;
    }
    // we get value part
    String sValues = pin_digitAnalog_val.substring(ind+1);
    
    // we separate the values
    ind = sValues.indexOf(",");
    if (ind < 0)   {
      LOG_ERROR(F("pinWrite cmd needs 3 values"));
      return 2;
    }

    // we get 1st value
    String sValue = sValues.substring(0,ind);
    // value must be an int
    int iValue1 = sValue.toInt();
    // toInt will return 0, if it is not an int
    if ( (iValue1 == 0) && ( ! sValue.equals("0")) )   {
      LOG_ERROR(F("pinWrite cmd: value 1 must be integer"));
      return 21;
    }
    else if ((iValue1 < 0) || (20 < iValue1))   {   // note that the value 20 is not accurate
      LOG_ERROR(F("pinWrite cmd: value 1 must be compatible with pin num"));
      return 3;
    }

    // we get the 2 last values
    sValues = sValues.substring(ind+1);
    // we separate the 2 last values
    ind = sValues.indexOf(",");
    if (ind < 0)   {
      LOG_ERROR(F("pinWrite cmd needs 3 values"));
      return 31;
    }

    // we get 2nd value
    sValue = sValues.substring(0,ind);
    // value must be an int
    int iValue2 = sValue.toInt();
    // toInt will return 0, if it is not an int
    if ( (iValue2 == 0) && ( ! sValue.equals("0")) )   {
      LOG_ERROR(F("pinWrite cmd: value 2 must be integer"));
      return 4;
    }
    else if ((iValue2 < 1) || (2 < iValue2))   {
      LOG_ERROR(F("pinWrite cmd: value 2 must be digital/analog=1/2"));
      return 5;
    }

    // we get 3rd value
    sValue = sValues.substring(ind+1);
    // value must be an int
    int iValue3 = sValue.toInt();
    // toInt will return 0, if it is not an int
    if ( (iValue3 == 0) && ( ! sValue.equals("0")) )   {
      LOG_ERROR(F("pinWrite cmd: value 3 must be integer"));
      return 6;
    }
    else if (iValue3 < 0)   {
      LOG_ERROR(F("pinWrite cmd: value 3 must be >=0"));
      return 7;
    }

    // ok, so we can make command
    if (iValue2 == 1)
      digitalWrite(iValue1,iValue3);
    else   // iValue2 == 2
      analogWrite(iValue1,iValue3);

    // I send back OK msg
    String msg2py = msg2pyStart + "pinWrite/OK" + msg2pyEnd;
    SERIAL_MSG.print(msg2py);
    
    return 0;
}

