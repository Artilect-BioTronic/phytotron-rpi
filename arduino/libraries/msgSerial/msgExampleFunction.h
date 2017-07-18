#ifndef MSGEXAMPLEFUNCTION_H
#define MSGEXAMPLEFUNCTION_H

#include "Arduino.h"

//#include "SD.h"
//#include "SdFat.h"
//extern SdFat SD;

class CommandList;

/*---------------------------------------------------------------*/
/*                                          */
/*---------------------------------------------------------------*/

int sendMessageStatus(const CommandList &aCL, const String& dumb);
int sendFakeVal(const CommandList& aCL, const String& dumb);
int ledBlinkTime(const CommandList &aCL, const String& dumb);  // uses  int blinkTime=1000;    defined in .cpp
int switchLed(const CommandList& aCL, const String& dumb);

int switchLed13(const CommandList& aCL, const String& dumb);
int sendMultiValue(const CommandList& aCL, const String& dumb);
int sendFakeDate(const CommandList& aCL, const String& dumb);
int updateGlobalVar(const CommandList& aCL, const String& dumb);

int fakeValue(int deb=0, int fin=1023);
String fakeDate(int iFormat=1);

#endif   // MSGEXAMPLEFUNCTION_H
