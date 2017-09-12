#ifndef MSGSERIAL_H
#define MSGSERIAL_H

#include "Arduino.h"


class CommandList;

class Command {
public:
    const String cmdName;
    const String cmdFormat;
    const String cmdLimit;
    int (*cmdFunction)(const CommandList& aCL, Command& aCmd, const String& aTopicValue) ;
    Command(const char *cn, int (*cf)(const CommandList&, Command&, const String&),
            const char *cfo ="", const char *cl ="") :
        cmdName(cn), cmdFormat(cfo), cmdLimit(cl), cmdFunction(cf) {}

//    int _ErrorNum;   // 0 if no error while processing command
};

extern String msg2pyStart, msg2mqttStart, msg2pyEnd, prefAT, prefDO ;


// list of available commands (user) that the arduino will accept
// ex  that you define in main sketch.ino
/*
Commande cmdos[] = {
  Commande("SendValue", &sendMessageStatus),
  Commande("s",         &sendMessageStatus),
  Commande("ledBlink",  &ledBlinkTime),
  Commande("lit1/switch",    &switchLed)
};
int cmdosSize = sizeof(cmdos) / sizeof(Commande);

// list of available commands (system ctrl) that the arduino will accept
// ex  that you define in main sketch.ino
Commande cmds[] = {
  Commande("idSketch",  &sendSketchId),
  Commande("idBuild",   &sendSketchBuild),
  Commande("listCmdAT", &sendListCmdAT),
  Commande("listCmdDO", &sendListCmdDO),
  Commande("listPin",   &sendListPin),
  Commande("pinMode",   &cmdPinMode),
  Commande("pinRead",   &cmdPinRead),
  Commande("pinWrite",  &cmdPinWrite)
};
int cmdsSize = sizeof(cmds) / sizeof(Commande);
*/

// you need to read serial and empty buffer into inputMessage
// for this you can call method  checkMessageReceived  in  loop function, like that
/*
  serListener.checkMessageReceived();
*/

// the macro will replace the array var  with its size and the array
#define SIZE_OF_TAB_AND_TAB(tab)    sizeof(tab)/sizeof(tab[0]), tab
#define SIZE_OF_TAB(tab)            sizeof(tab)/sizeof(tab[0])


class SerialListener;
class ParsedCommand;

class CommandList
{
private:
    char _cmdSeparator;
    char _argSeparator;

    const uint8_t _nbObjects;
    Command *_arrayCmd;
    Stream * _pStream;   /// provided by SerialListener
    SerialListener * _pSerialListener;
    const String _nameListCmd;
    const String _prefix;
    const char _cmdEnd;

public:
    uint8_t getNbCmd() const  { return _nbObjects; }
    Command* getCmd(uint8_t i);
    char getCmdSeparator()  const   { return _cmdSeparator; }
    char getArgSeparator()  const   { return _argSeparator; }
    String getCmdPrefix() const     { return _prefix; }
    char getCmdEnd() const          { return _cmdEnd; }
    Stream* getStream()             { return _pStream;}
    SerialListener* getSerialListener() const  {return _pSerialListener;}

    String getCommand(const String& aCmdVal) const ;

    void setStream(Stream * apStream)   {_pStream = apStream;}
    void setSerialListener(SerialListener * apSerialListener)   {_pSerialListener = apSerialListener;}

    int displayListCmd(String& aNameCL, String& asMode)  const;

    size_t msgPrint(const String& aMsg) const;
    size_t msgOK(const String& aStrCmd, const String& aMsg) const;
    size_t msgOK(const String& aStrCmd, int aMsg) const;
    size_t msgKO(const String& aStrCmd, const String& aMsg) const;
    size_t msgKO(const String& aStrCmd, int aMsg) const;
    size_t msgError(const String& aMsg) const;
//    int returnKO(Command& aCmd) const ;
    int returnKO(Command& aCmd, ParsedCommand& aPC) const ;

    CommandList(const String &nameListCmd, const String &prefix,
                const int nbObjects, Command arrayCmd[],
                char cmdSeparator=':', char argSeparator=',', char cmdEnd='\n');

    bool checkMessageReceived(String aInputMessage);
    bool readInternalMessage(String aInternalNudeMessage);

    int verifyFormatMsg(Command& aCmd, const String& sOnOff);
};

class SerialListener
{
private:
    Stream & _stream;
    uint8_t _nbObjects;
    static const uint8_t _nbObjectsMax = 10;
    CommandList* _arrayObjects[_nbObjectsMax];
    String _inputMessage;
    bool _inputMessageReceived;

    String _cmdEnds;
    unsigned long _timeLastIncomingChar;
    unsigned int _timeOut;

public:
    SerialListener(Stream &aStream);
    bool addCmdList(CommandList& cmdL);
    void copyBuffer();
    void checkMessageReceived();

    int displayListCmd(String& aNameCL, String& asMode)  const;
};

class ParsedCommand
{
public:
    enum  ERROR_PC { NO_ERROR=0, ERROR_IN_CODE, TOO_LONG, NB_ARG_DIFFERENT, INDEX_ABOVE_NB_ARG,
                     TOO_MANY_ARG, INT_REQUIRED, FLOAT_REQUIRED, UNKNOWN_FMT,
                     CODE_BAD_FMT, LIMIT_BAD };
    static const int _maxLengthString = 127;
    static const int _maxNbArg = 5;
    static const char PC_STRING[];
    static const char PC_CHAR_LIST[];
    static const char PC_INT[];
    static const char PC_FLOAT[];
    static const char PC_LIM_ANY_OPT[];

private:
    const CommandList& _cmdList;
    const Command& _cmd;
    const String& _strCmdArg;
    uint8_t _indicesCmd[_maxNbArg+1];
    uint8_t _indicesFmt[_maxNbArg+1];
    uint8_t _indicesLim[_maxNbArg+1];
    ERROR_PC _numError;
    String   _strError;
    uint8_t _nbArg;

    boolean checkLimit1Str(const String& aArg, const String &aFmt, const String& aLim);
    boolean checkLimit1Int(const String& aArg, const String &aFmt, const String& aLim);
    boolean checkLimit1Float(const String& aArg, const String &aFmt, const String& aLim);
    boolean checkLimitSeveralChar(const String& aArg, const String &aFmt, const String& aLim);

public:
    ParsedCommand(const CommandList& aCmdList, const Command &aCmd, const String& aStrCmdArg);

    ParsedCommand::ERROR_PC setError(ERROR_PC nerr)   {_numError=nerr; return nerr;}
    ParsedCommand::ERROR_PC getError()  const  {return _numError;}
    String getErrorStr() const  {return _strError;}
    boolean hasError()  const   {return _numError != NO_ERROR;}

    ERROR_PC verifyFormatMsg(Command& aCmd, const String& aStrCmdArg) ;
    ParsedCommand::ERROR_PC checkType(const String& aArg, const String& aFmt);
    ParsedCommand::ERROR_PC checkLimit(const String& aArg, const String &aFmt, const String& aLim);

    int splitString(const String& aStrCmdArg, uint8_t aIndices[_maxNbArg],
                    uint8_t& aNbArg, int aIndStart=0);
    String getArgStrNum(uint8_t anum);
    String getFmtNum(uint8_t anum) ;
    String getLimNum(uint8_t anum) ;

    int getValueInt(int anum);
    float getValueFloat(int anum);
    String getValueStr(int anum);

};


#endif // MSGSERIAL_H
