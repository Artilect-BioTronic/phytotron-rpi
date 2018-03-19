#include "Arduino.h"

#include "msgSerial.h"

#define LOG_DEBUG(str)   Serial.println(str)
//#define LOG_ERROR(str)   SERIAL_MSG.println(str)

// definition of static variable
const char ParsedCommand::PC_STRING[]     = "s";
const char ParsedCommand::PC_CHAR_LIST[]  = "cc";
const char ParsedCommand::PC_INT[]        = "i";
const char ParsedCommand::PC_FLOAT[]      = "f";
const char ParsedCommand::PC_LIM_ANY_OPT[]= "*";


String CommandList::getCommand(const String& aCmdVal) const   {
    // the aCmdVal contains  command:val   (with val=arg1,arg2 ...)
    int ind = aCmdVal.indexOf(getCmdSeparator());

    // if there is no val, then no ':', then ind=-1,
    if (ind < 0)   {
      return aCmdVal;    //   then whole string is the command
    }

    // we get command part
    return aCmdVal.substring(0,ind);
}

size_t CommandList::msgPrint(const String& aMsg) const  {
    String msg = _prefix + aMsg + _cmdEnd;
    return _pStream->print(msg);
}

size_t CommandList::msgError(const String& aMsg) const  {
    return _pStream->println(aMsg);
}

size_t CommandList::msgOK(const String& aStrCmd, const String& aMsg) const   {
    String msg = getCommand(aStrCmd) + "/OK:" + aMsg;
    return msgPrint(msg);
}
size_t CommandList::msgOK(const String& aStrCmd, int cr) const   {
    return msgOK(aStrCmd, String(cr) );
}

size_t CommandList::msgKO(const String& aStrCmd, const String& aMsg) const {
    String msg = getCommand(aStrCmd) + "/KO:" + aMsg;
    return msgPrint(msg);
}
size_t CommandList::msgKO(const String& aStrCmd, int cr) const {
    return msgKO(aStrCmd, String(cr) );
}

int CommandList::returnKO(Command& aCmd, ParsedCommand &aPC) const {
    msgError(aPC.getErrorStr());
    msgPrint(aCmd.cmdName + "/KO:" + aPC.getError());
    return aPC.getError();
}


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
                         const int nbObjects, Command arrayCmd[],
                         char cmdSeparator, char argSeparator, char cmdEnd):
    _nameListCmd(nameListCmd), _prefix(prefix), _nbObjects(nbObjects), _arrayCmd(arrayCmd),
    _pStream(NULL), _pSerialListener(NULL),
    _cmdSeparator(cmdSeparator), _argSeparator(argSeparator), _cmdEnd(cmdEnd)
{}

SerialListener::SerialListener(Stream &aStream): _stream(aStream), _nbObjects(0),
    _inputMessage(""), _inputMessageReceived(false), _cmdEnds(""),
    _timeLastIncomingChar(0), _timeOut(100)
{}


/**
 * @brief CommandList::checkMessageReceived check if a message is a command
 *
 * SerialListener checks a message is incoming, then it calls checkMessageReceived
 * to check if it is adressed to this CommandList
 * checkMessageReceived calls the callback command
 * @param aInputMessage
 * @return true if cmd has been found; even if the cmd fails
 */
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
                int cr = _arrayCmd[i].cmdFunction(*this, _arrayCmd[i], nudeMessage);
            }
        }
        if (cmdNotFound)  {
            msgError(_nameListCmd + F(" cmd not recognized ! :") + command);
            return false;
        }
        return true;
    }
    else
        return false;
}

/**
 * @brief CommandList::readInternalMessage execute command given as internal message
 *
 * if you want to execute a command internally in arduino, you cannot access the callback directly
 * you have to use this readInternalMessage
 * @param aInternalNudeMessage  String containing cmd, but without prefix & end used through SerialListener
 * @return true if cmd has been found; even if the cmd fails
 */
bool CommandList::readInternalMessage(const String& aInternalNudeMessage)
{
    // check if CommandList is linked to a SerialListener
    if (_pSerialListener == NULL)
        return false;

    // we extract the cmd name
    String command = "";
    int iCmdSeparator = aInternalNudeMessage.indexOf(_cmdSeparator);
    if (iCmdSeparator > 0)
        command = aInternalNudeMessage.substring(0, iCmdSeparator);
    else   {
        // if there is no arg => no separator,  indexOf returns -1
        // so cmd goes from prefix to end of string
        command = aInternalNudeMessage;
    }

    // We search the command among the list
    bool cmdNotFound = true;
    for (int i=0; i<_nbObjects && cmdNotFound; i++)
    {
        if (command == _arrayCmd[i].cmdName)
        {
            cmdNotFound = false;
            int cr = _arrayCmd[i].cmdFunction(*this, _arrayCmd[i], aInternalNudeMessage);
        }
    }
    if (cmdNotFound)  {
        msgError(_nameListCmd + F(" internal cmd not recognized ! :") + command);
        return false;
    }
    return true;
}

/**
 * @brief CommandList::displayListCmd  display the list of commands available
 * @param aNameCL  selects which CommandList is to be displayed
 * @param asMode   short: display only commands, full: display the argument format
 * @return   0  if there is no pb
 */
int CommandList::displayListCmd(String& aNameCL, String& asMode)  const {
    if ( (aNameCL != "") && (aNameCL != _nameListCmd) )
        return 0;   // this CommandList is not to be displayed

    for (int i=0; i<_nbObjects; i++)   {
        if (asMode == F("short"))
            msgPrint(String(F("list:")) + _arrayCmd[i].cmdName);
        else
            msgPrint(String(F("list:")) + _arrayCmd[i].cmdName +" "
                     + _arrayCmd[i].cmdFormat +" "+ _arrayCmd[i].cmdLimit);
    }
    return 0;
}


/**
 * @brief SerialListener::addCmdList  add CommandList that will be managed by  SerialListener
 * @param cmdL
 * @return
 */
bool SerialListener::addCmdList(CommandList &cmdL)   {
    if ( _nbObjects < _nbObjectsMax )   {
        _arrayObjects[_nbObjects] = &cmdL;
        _nbObjects++;
        _cmdEnds += cmdL.getCmdEnd();
        cmdL.setStream(&_stream);
        cmdL.setSerialListener(this);
        return true;
    }
    else
        return false;
}

/**
 * @brief SerialListener::copyBuffer  copy _stream buffer into SerialListener buffer
 *
 * copy _stream buffer  into  SerialListener buffer as soon as possible, this is
 * called by checkMessageReceived.
 * The copy is stopped when we encounter end msg char; the next msg stays in _stream buffer
 */
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

/**
 * @brief SerialListener::displayListCmd  display the list of commands available
 * @param aNameCL  selects which CommandList is to be displayed
 * @param asMode   short: display only commands, full: display the argument format
 * @return   0  if there is no pb
 */
int SerialListener::displayListCmd(String& aNameCL, String& asMode)  const {
    int cr=0, cr2=0;
    for (int i=0; i<_nbObjects; i++)   {
        cr2 = _arrayObjects[i]->displayListCmd(aNameCL, asMode);
        if (cr2 != 0)   cr = cr2;
    }
    return cr;
}


// ParsedCommand class
//--------------------

ParsedCommand::ParsedCommand(const CommandList& aCmdList, const Command& aCmd, const String& aStrCmdArg) :
    _cmdList(aCmdList), _cmd(aCmd), _strCmdArg(aStrCmdArg),
    _numError(NO_ERROR), _strError("")
{
    if ( (_strCmdArg.length()     > _maxLengthString) ||
         (_cmd.cmdFormat.length() > _maxLengthString) ||
         (_cmd.cmdLimit.length()  > _maxLengthString)    )   {
        _strError = F("string too long");
        _numError = TOO_LONG;
    }

    // filling indices

    // ie: _cmd.cmdFormat = "d,d,s"   ->   indicesFmt = {0,2,4,6,0,0,6}
    splitString(_cmd.cmdFormat, _indicesFmt, _nbArg);

    // _strCmdArg split begining after _cmdSeparator
    // ie: _strCmdArg = "cmd:1,2,cd"   ->   indicesCmd = {4,6,8,11,0,0,11}
    int indStart = _strCmdArg.indexOf(_cmdList.getCmdSeparator())+1;
    // if CmdSeparator is missing   indexOf returns -1,  we restore this value
    if (indStart <= 0)
        indStart = -1;
    uint8_t nbTemp=0;
    splitString(_strCmdArg, _indicesCmd, nbTemp, indStart);
    // we must have nbTemp == _nbArg; except if ... fmt is PC_CHAR_LIST, then the single arg may be optional
    if (nbTemp != _nbArg)
    {
        // if format is a single string, it can contain arg separator "," ; we avoid string from being split in several
        if ( (_cmd.cmdFormat == PC_STRING) && (_nbArg == 1) && (nbTemp > 1) )   {
            // we reinterpret incoming _strCmdArg  as a single string
            // we change the position of the end of 1st arg (=start of virtual 2nd arg)
            _indicesCmd[1] = _strCmdArg.length()+1;
        }
        else if (  (_cmd.cmdFormat == PC_CHAR_LIST) && (_nbArg == 1) && (nbTemp == 0)  )
        {
            // if  fmt is PC_CHAR_LIST, then the single arg may be optional
        }
        else if ( (_cmd.cmdFormat == PC_STRING) &&
                  (_nbArg == 1) && (nbTemp == 0) && (_cmd.cmdLimit == PC_LIM_ANY_OPT) )
        {
            // if  fmt is PC_STRING, then sometimes the single arg is optional
            // user must express it by giving a lim  PC_LIM_ANY_OPT   (ie: "*")
        }
        else
        {
            _numError = NB_ARG_DIFFERENT;
            _strError = String("Format: ")+ _cmd.cmdFormat +" requires "+ _nbArg +" arguments";
        }
    }

    splitString(_cmd.cmdLimit, _indicesLim, nbTemp);
    // we test error, but only if there is not already an error
    // cmdLimit is optional, it is authorized to be empty; otherwise nb of arg must be the same
    if ( (_numError == NO_ERROR) && (nbTemp != _nbArg) && (nbTemp != 0) )   {
        _numError = NB_ARG_DIFFERENT;
        _strError = String("Format: ")+ _cmd.cmdFormat +" incompatible with nb of limits "+ _cmd.cmdLimit;
    }
}

int ParsedCommand::splitString(const String& aStrCmdArg, uint8_t aIndices[_maxNbArg],
                               uint8_t& aNbArg, int aIndStart)   {
    // DEBUG
//    Serial.println(String("debug:") + aStrCmdArg +" starting at: "+ aIndStart);

    aNbArg = 0;
    int indEndOfStrArg = aStrCmdArg.length()-1;

    // We check case if there is no arg
    if ( aIndStart < 0 || indEndOfStrArg < aIndStart )   {
        for (int i=0; i<=_maxNbArg; i++)   aIndices[i] = 0;
        aIndices[1] = 1;    // so the cmd getArg(1) will send back empty arg
        return 0;
    }

    int ind = aIndStart-1;
    do   {
        aIndices[aNbArg] = ind+1;
        aNbArg++;
        if (ind < indEndOfStrArg)
            // indexOf returns -1 if not found
            ind = aStrCmdArg.indexOf(_cmdList.getArgSeparator(), ind+1);
        else
            ind = -1;
    }   while ((ind >= 0) && (aNbArg < _maxNbArg));

    // test if there were too many arg
    if ((ind >= 0) && (aNbArg == _maxNbArg))   {
        aIndices[aNbArg] = ind+1;
        _numError = TOO_MANY_ARG;
        _strError = String("too many arg in string: ")+ aStrCmdArg;
        return -1;
    }

    // we set ind for a virtual next arg; we set it at  last possible index +1
    aIndices[aNbArg] = indEndOfStrArg+2;

    /*
    // DEBUG
    String darg = "debug indices: "+ String(aIndices[0]);
    for (int i=1; i<_maxNbArg+1; i++)   {
        darg += String(",")+ aIndices[i];
    }
    Serial.println(darg);
    */
    return 0;
}

/**
 * @brief ParsedCommand::getArgStrNum get arg num anum in string _strCmdArg
 *
 * the arg is returned as String
 * @param anum  num of arg to get (starts with 1)
 * @return String  arg of num anum in string _strCmdArg
 */
String ParsedCommand::getArgStrNum(uint8_t anum)    {
    if (_numError != NO_ERROR)   return "";   // error may have been detected by constructor
    if (anum > _nbArg)   {
        _numError = INDEX_ABOVE_NB_ARG;
        return "";
    }
    return _strCmdArg.substring(_indicesCmd[anum-1], _indicesCmd[anum-1+1]-1);
}

String ParsedCommand::getFmtNum(uint8_t anum)    {
    if (_numError != NO_ERROR)   return "";   // error may have been detected by constructor
    if (anum > _nbArg)   {
        _numError = INDEX_ABOVE_NB_ARG;
        return "";
    }
    return _cmd.cmdFormat.substring(_indicesFmt[anum-1], _indicesFmt[anum-1+1]-1);
}

String ParsedCommand::getLimNum(uint8_t anum)    {
    if (_numError != NO_ERROR)   return "";   // error may have been detected by constructor
    if (anum > _nbArg)   {
        _numError = INDEX_ABOVE_NB_ARG;
        return "";
    }
    return _cmd.cmdLimit.substring(_indicesLim[anum-1], _indicesLim[anum-1+1]-1);
}

int ParsedCommand::getValueInt(int anum)   {
    // test input
    if ( (anum < 1) || (_nbArg < anum) )   {
        // if an error has been detected already, we dont erase the former one
        if (_numError == NO_ERROR)  {
            _numError = INDEX_ABOVE_NB_ARG;
            _strError = String(F("index num: ")) + anum +" out of range";
        }
        return -1;
    }

    // check type of arg that is asked
    if (getFmtNum(anum) != PC_INT)   {
        // if an error has been detected already, we dont erase the former one
        if (_numError == NO_ERROR)  {
            _numError = CODE_BAD_FMT;
            _strError = String(F("index num: ")) + anum +" code asks for wrong format";
        }
        return -1;
    }

    String arg = getArgStrNum(anum);
    return arg.toInt();

}

float ParsedCommand::getValueFloat(int anum)   {
    // test input
    if ( (anum < 1) || (_nbArg < anum) )   {
        // if an error has been detected already, we dont erase the former one
        if (_numError == NO_ERROR)  {
            _numError = INDEX_ABOVE_NB_ARG;
            _strError = String(F("index num: ")) + anum +" out of range";
        }
        return -1;
    }

    // check type of arg that is asked
    if (getFmtNum(anum) != PC_FLOAT)   {
        // if an error has been detected already, we dont erase the former one
        if (_numError == NO_ERROR)  {
            _numError = CODE_BAD_FMT;
            _strError = String(F("index num: ")) + anum +" code asks for wrong format";
        }
        return -1;
    }

    String arg = getArgStrNum(anum);
    return arg.toFloat();

}

String ParsedCommand::getValueStr(int anum)   {
    // test input
    if ( (anum < 1) || (_nbArg < anum) )   {
        // if an error has been detected already, we dont erase the former one
        if (_numError == NO_ERROR)  {
            _numError = INDEX_ABOVE_NB_ARG;
            _strError = String(F("index num: ")) + anum +" out of range";
        }
        return "";
    }

    // check type of arg that is asked
    if ( (getFmtNum(anum) != PC_STRING) && (getFmtNum(anum) != PC_CHAR_LIST) )  {
        // if an error has been detected already, we dont erase the former one
        if (_numError == NO_ERROR)  {
            _numError = CODE_BAD_FMT;
            _strError = String(F("index num: ")) + anum +" code asks for wrong format";
        }
        return "";
    }

    return getArgStrNum(anum);

}

ParsedCommand::ERROR_PC ParsedCommand::checkType(const String& aArg, const String& aFmt) {
    // if it is s String, any input is valid
    if (aFmt == PC_STRING)
        return NO_ERROR;
    // if it is  cc , it is a string whose letters are optional (ie ls | ls r | ls ard )
    //  all content in aArg may be possible, the validity will be checked with limit
    else if (aFmt == PC_CHAR_LIST)
        return NO_ERROR;
    // if it is i  integer, we try and convert with toInt
    else if (aFmt == PC_INT)
    {
        int iValue = aArg.toInt();
        // toInt will return 0, if it is not an int
        if ( (iValue == 0) && ( ! aArg.equals("0")) )   {
          _strError = String(F("value: ")) + aArg + F(" must be integer");
          _numError = INT_REQUIRED;
          return INT_REQUIRED;
        }
    }
    else if (aFmt == PC_FLOAT)
    {
        String notFloat = F("not a float");
        double dValue = aArg.toFloat();
        // toFloat will return 0.0, if it is not a float
        if ( (dValue == notFloat.toFloat()) && ( ! aArg.equals("0."))
                                            && ( ! aArg.equals("0"))  )   {
          _strError = String(F("value: ")) + aArg + F(" must be float");
          _numError = INT_REQUIRED;
          return INT_REQUIRED;
        }
    }
    else
    {
        _strError = String(F("format: ")) + aFmt + F(" is not known");
        _numError = UNKNOWN_FMT;
        return UNKNOWN_FMT;
    }

    return NO_ERROR;
}

ParsedCommand::ERROR_PC ParsedCommand::checkLimit(const String& aArg,
                                                  const String& aFmt, const String& aLim)  {
    // indication of limit is optional,  * means all is possible
    if (aLim == "*")   return NO_ERROR;
    // empty lim, I accept as = "*"
    // yet   if aFmt == PC_CHAR_LIST  then  aLim is mandatory
    if ( (aLim == "") && (aFmt != PC_CHAR_LIST) )  return NO_ERROR;

    // check aFmt=PC_CHAR_LIST : a string made of several optional char
    // this case is different because there is only one pattern
    if (aFmt == PC_CHAR_LIST)   {
        if ( checkLimitSeveralChar(aArg, aFmt, aLim) )
            return NO_ERROR;
        else
            return _numError;
    }

    boolean limOK = false;

    // if aLim = "ON|OFF", there is 2 values for singleLim: ON  or  OFF
    // we parse aLim and test each possible  singleLim
    int indStart = 0;
    int indSeparator=0;
    do  {
        indSeparator = aLim.indexOf("|", indStart);

        // if there is no separator, indexOf returns -1
        int indEnd = indSeparator;
        if (indEnd < 0)
            indEnd = aLim.length();
        //
        String singleLim = aLim.substring(indStart, indEnd);
        // if it is string
        if (aFmt == PC_STRING)   {
            limOK = checkLimit1Str(aArg, aFmt, singleLim);
        }
        else if (aFmt == PC_INT)   {
            limOK = checkLimit1Int(aArg, aFmt, singleLim);
        }
        else if (aFmt == PC_FLOAT)   {
            limOK = checkLimit1Float(aArg, aFmt, singleLim);
        }
        else   {
            _strError = F("checkLimit does not recognize format");
            return setError(UNKNOWN_FMT);
        }
        indStart = indSeparator + 1;
    }
    while ( (indSeparator > 0) && (! limOK) );

    if (! limOK)   {
        // aArg  dont respect any limit pattern in aLim
        _strError = String("arg: ") + aArg +" incompatible with limit: "+ aLim;
        _numError = LIMIT_BAD;
        return LIMIT_BAD;
    }
    else
        return NO_ERROR;
}

boolean ParsedCommand::checkLimit1Str(const String& aArg,
                                      const String& aFmt, const String& aLim)  {
    // DEBUG
//    Serial.println(String("arg: ")+ aArg +" limit: "+ aLim);

    if (aLim == "*")
        return true;

    if (aArg == aLim)
        return true;
    else
        return false;
}

boolean ParsedCommand::checkLimit1Int(const String& aArg,
                                          const String& aFmt, const String& aLim)  {
    // DEBUG
//    Serial.println(String("arg: ")+ aArg +" limit: "+ aLim);

    if (aLim == "*")
        return true;

    if (aArg == aLim)
        return true;

    // we check if the pattern is start - end  (ie 1-9)
    int hyphen = aLim.indexOf("-");
    // if pattern is of type  -10-10   (ie: from -10 to +10)
    //   then we look for 2nd hyphen
    if (hyphen == 0)
        hyphen = aLim.indexOf("-", 1);

    if (hyphen > 0)   {
        String sstart = aLim.substring(0,hyphen);
        String send = aLim.substring(hyphen+1);
        int start = sstart.toInt();
        int end = send.toInt();
        int iArg = aArg.toInt();
        // start and end must be integers.  if toInt fails, it returns 0
        if (  ( (start == 0) && (sstart != "0") )  ||
              ( (end   == 0) && (send   != "0") )  ||
              ( (iArg  == 0) && (aArg   != "0") )     )   {
            _strError = String("error in limit pattern with hyphen: ") + aLim;
            _numError = LIMIT_BAD;
            return false;
        }
        if ( (start <= iArg) && (iArg <= end) )
            return true;
    }

    return false;
}
boolean ParsedCommand::checkLimit1Float(const String& aArg,
                                        const String& aFmt, const String& aLim)  {
    // DEBUG
//    Serial.println(String("arg: ")+ aArg +" limit: "+ aLim);

    if (aLim == "*")
        return true;

    if (aArg == aLim)
        return true;

    // we check if the pattern is start - end  (ie 1-9)
    int hyphen = aLim.indexOf("-");
    if (hyphen > 0)   {
        String notFloat = F("not a float");
        double dNoFloat = notFloat.toFloat();
        String sstart = aLim.substring(0,hyphen);
        String send = aLim.substring(hyphen+1);
        double start = sstart.toFloat();
        double end = send.toFloat();
        double dArg = aArg.toFloat();
        // start and end must be integers.  if toFloat fails, it returns 0.
        if (  ( (start == dNoFloat) && ( ! sstart.startsWith("0")) )  ||
              ( (end   == dNoFloat) && ( ! send.startsWith("0")  ) )  ||
              ( (dArg  == dNoFloat) && ( ! aArg.startsWith("0")  ) )     )   {
            _strError = String("error in limit pattern with hyphen: ") + aLim;
            _numError = LIMIT_BAD;
            return false;
        }
        if ( (start <= dArg) && (dArg <= end) )
            return true;
    }

    return false;
}

boolean ParsedCommand::checkLimitSeveralChar(const String& aArg,
                                      const String& aFmt, const String& aLim)  {
    if (aLim == "")   {
        _strError = String("empty lim, whereas it is mandatory with format: ") + aFmt;
        _numError = LIMIT_BAD;
        return false;
    }

    // all char in aArg  must be present in aLim
    for (int i=0; i<aArg.length(); i++)   {
        // if  char is not  present  (indexOf returns -1)
        if ( aLim.indexOf(aArg.charAt(i)) < 0 )   {
            _strError = String("char:") + aArg.charAt(i) +" must be one of: " + aLim;
            _numError = LIMIT_BAD;
            return false;
        }
    }

    return true;
}


ParsedCommand::ERROR_PC ParsedCommand::verifyFormatMsg(Command& aCmd, const String& aStrCmdArg)   {
    // error may have been detected by constructor
    if (_numError != NO_ERROR)   return _numError;
    // if cmdArg, cmdFormat or cmdLimit have different nb, this is detected by constructor

    // DEBUG
/*    String darg = "indices strcmdarg: "+ String(_indicesCmd[0]);
    String dfmt = "indices strfmt: "+ String(_indicesFmt[0]);
    String dlim = "indices strlim: "+ String(_indicesLim[0]);
    for (int i=1; i<_nbArg+1; i++)   {
        darg += String(",")+ _indicesCmd[i];
        dfmt += String(",")+ _indicesFmt[i];
        dlim += String(",")+ _indicesLim[i];
    }
    Serial.println(darg);
    Serial.println(dfmt);
    Serial.println(dlim);
*/
    for (int i=1; i<=_nbArg; i++)   {
        // check type of arg
        if (checkType(getArgStrNum(i), getFmtNum(i)) != NO_ERROR)   return _numError;
        if (checkLimit(getArgStrNum(i), getFmtNum(i), getLimNum(i)) != NO_ERROR)   return _numError;
    }

    return NO_ERROR;
}


