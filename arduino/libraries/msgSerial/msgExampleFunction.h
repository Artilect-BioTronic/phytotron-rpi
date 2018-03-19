#ifndef MSGEXAMPLEFUNCTION_H
#define MSGEXAMPLEFUNCTION_H

#include "Arduino.h"
#include "msgSerial.h"


extern int blinkTime;   // used by blinkTime function

/*---------------------------------------------------------------*/
/*                                          */
/*---------------------------------------------------------------*/

int sendMessageStatus   (const CommandList &aCL, Command& aCmd, const String& dumb);
int sendFakeVal     (const CommandList& aCL, Command& aCmd, const String& dumb);
int ledBlinkTime    (const CommandList& aCL, Command& aCmd, const String& aInput);  // uses  int blinkTime=1000;    defined in .cpp
int switchLed       (const CommandList& aCL, Command& aCmd, const String& dumb);

int sendBackArg_s   (const CommandList& aCL, Command& aCmd, const String& aInput);
int sendBackArg_ss  (const CommandList& aCL, Command& aCmd, const String& aInput);
int sendBackArg_is  (const CommandList& aCL, Command& aCmd, const String& aInput);
int sendBackArg_fs  (const CommandList& aCL, Command& aCmd, const String& aInput);

int switchLed13     (const CommandList& aCL, Command& aCmd, const String& dumb);
int sendMultiValue  (const CommandList& aCL, Command& aCmd, const String& dumb);
int sendFakeDate    (const CommandList& aCL, Command& aCmd, const String& dumb);
int updateGlobalVar (const CommandList& aCL, Command& aCmd, const String& dumb);

int fakeValue(int deb=0, int fin=1023);
String fakeDate(int iFormat=1);
int getSensorValue();

// You can keep all those functions here
//   they DO NOT take program space if you dont call them in main sketch.ino
//   that means if they are not included in your cmdos and cmds  arrays

// list of available commands (user) that the arduino will accept
//   there is a list of functions in  msgExampleFunction.h

// list of available commands (system ctrl) that the arduino will accept
int sendSketchId    (const CommandList& aCL, Command& aCmd, const String& aInput);
int sendSketchBuild (const CommandList& aCL, Command& aCmd, const String& aInput);
int sendListCmd     (const CommandList& aCL, Command& aCmd, const String& aInput);
int sendListPin     (const CommandList& aCL, Command& aCmd, const String& aInput);
int cmdPinMode      (const CommandList& aCL, Command& aCmd, const String& aInput);
int cmdPinRead      (const CommandList& aCL, Command& aCmd, const String& aInput);
int cmdPinWrite     (const CommandList& aCL, Command& aCmd, const String& aInput);


class SketchInfo   {
private:
    static PROGMEM const char S_UNKNOWN[];
public:
    SketchInfo(): file_(S_UNKNOWN), date_(S_UNKNOWN),time_(S_UNKNOWN),listPin_("") {}
    void setFile(const String &aFullFilename);
    void setDate(const String& arg)   {date_ = arg;}
    void setTime(const String& arg)   {time_ = arg;}
    void setFileDateTime(const String &aFullFilename, const String& aDate, const String& aTime);
    void addListPin(const String &arg)   {listPin_ = listPin_ +"\n"+ arg;}
    String getFile()   {return file_;}
    String getDate()   {return String(date_);}
    String getTime()   {return String(time_);}
    String getListPin()   {return listPin_;}
private:
    String file_;
    String date_;
    String time_;
    String listPin_;
};
extern SketchInfo sketchInfo;


#endif   // MSGEXAMPLEFUNCTION_H
