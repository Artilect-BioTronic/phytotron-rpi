#ifndef MSGSERIAL_H
#define MSGSERIAL_H

#include "Arduino.h"
#include "msgExampleFunction.h"


#define SERIAL_MSG Serial1


class SketchInfo   {
public:
    SketchInfo(): file_("unknown"), date_("unknown"),time_("unknown") {}
    void setFile(const String &aFullFilename);
    void setDate(const String &arg)   {date_ = arg;}
    void setTime(const String &arg)   {time_ = arg;}
    void setFileDateTime(const String &aFullFilename, const String &aDate,const String &aTime);
    String getFile()   {return file_;}
    String getDate()   {return date_;}
    String getTime()   {return time_;}
private:
    String file_;
    String date_;
    String time_;
};
extern SketchInfo sketchInfo;

struct stListPin { int numPin; const char *namePin;
       stListPin(int n, const char *np) : numPin(n), namePin(np) {}
};

// listPin and listPinSize are defined in main sketch.ino
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
#define PIN_LED13 13

struct Command {
    const String cmdName;
    int (*cmdFunction)(const String& topic_value) ;
    Command(const char *cn, int (*cf)(const String&) ): cmdName(cn), cmdFunction(cf) {}
};

extern boolean inputMessageReceived;
extern String msg2pyStart, msg2mqttStart, msg2pyEnd, prefAT, prefDO ;

// You can keep all those functions here
//   they DO NOT take program space if you dont call them in main sketch.ino
//   that means if they are not included in your cmdos and cmds  arrays

// list of available commands (user) that the arduino will accept
//   there is a list of functions in  msgExampleFunction.h

// list of available commands (system ctrl) that the arduino will accept
int sendSketchId(const String& dumb);
int sendSketchBuild(const String& dumb);
int sendListCmdAT(const String& dumb);
int sendListCmdDO(const String& dumb);
int sendListPin(const String& dumb);
int cmdPinMode(const String& dumb);
int cmdPinRead(const String& dumb);
int cmdPinWrite(const String& dumb);

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


// functions to send back messages

size_t msgSPrint(const String& aMsg);
size_t msgSPrintln(const String& aMsg);
size_t msgSError(const String& aMsg);
String getCommand(const String& aCmdVal);


class CommandList
{
private:
    char _cmdSeparator;
    char _argSeparator;

    const uint8_t _nbObjects;
    const Command *_arrayCmd;
    const String _nameListCmd;
    const String _prefix;
    const char _cmdEnd;

public:
    uint8_t getNbCmd() { return _nbObjects; }
    Command* getCmd(uint8_t i);
    char getCmdEnd() { return _cmdEnd; }

    CommandList(const String &nameListCmd, const String &prefix,
                const int nbObjects, const Command arrayCmd[],
                char cmdSeparator=':', char argSeparator=',', char cmdEnd='\n');

    bool checkMessageReceived(String aInputMessage);
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
};

class CommandC
{
  private:
  static uint8_t nbObjects;
  static const uint8_t nbObjectsMax = 20;
  static CommandC* arrayObjects[nbObjectsMax];
  static int addCmdInArray(CommandC* pCmd);

  public:
  static uint8_t getNbObjects() {return nbObjects;}
  static CommandC* getObject(uint8_t i);

  const String cmdName;
  int (*cmdFunction)(const String& topic_value) ;
  CommandC(const char* cn, int (*cf)(const String&) ): cmdName(cn), cmdFunction(cf) {
    addCmdInArray(this);
  }
};

//void serialEventMFMQTT();

// because of bad communication, some messages may be stucked in
//   serial buffer. If so, we trace it
int  checkNoStuckMessageInBuffer() ;

// check if a message has been received and analyze it
// in your main sketch.ino, you have to update global var inputMessageReceived and inputMessage
// checkMessageReceived calls the function corresponding to  cmds/cmdos array
void checkMessageReceived();

int getSensorValue(); 


#endif // MSGSERIAL_H
