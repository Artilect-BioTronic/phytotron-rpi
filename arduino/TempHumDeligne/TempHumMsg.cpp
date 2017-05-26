#include "Arduino.h"

#include "TempHumMsg.h"

#include "msgSerial.h"
#include "msg2SDCard.h"

#include <DS1307RTC.h>

SerialListener serListenerTH(Serial);

Command cmdUserPhy[] = {
    Command("SV",                   &sendFakeVal),
    Command("csgn/humidity/cmd",    &updateHumCsgn),
    Command("csgn/temp/cmd",        &updateTempCsgn),
    Command("sendDate",             &sendDate)
};
CommandList cmdLUserPhy("cmdUser", "CM+", SIZE_OF_TAB(cmdUserPhy), cmdUserPhy );

// list of available commandes (system ctrl) that the arduino will accept
// example:  int sendSketchId(const String& dumb);

// list of available commands (user) that the arduino will accept

// list of available commands (system ctrl) that the arduino will accept
Command cmdSysPhy[] = {
    Command("idSketch",         &sendSketchId),
    Command("idBuild",          &sendSketchBuild),
    Command("srStayOpen",       &srStayOpen),    // :bob.txt,67  filenameDOS8.3 (short names), openMode (O_READ... specific to SdFat lib)
    Command("srPreOpen",        &srPreOpen),     // :prepare to open at each read/write (it is closed immediately after)
    Command("srClose",          &srClose),   // pas de param
    Command("srReadln",         &srReadln),  // pas de param
    Command("srWriteln",        &srWriteln), // :nouvelle ligne
    Command("srReadNchar",      &srReadNchar),   // :nbchar
    Command("srMove",           &srMove),    // :str2search
    Command("srDump2",           &srDump2),  // pas de param
    Command("srLs",             &srLs),      // :15  donner les options du ls
    Command("srRename",         &srRename),  // :/adir/old,new
    Command("srMkdir",          &srMkdir),   // :/adir
    Command("srRemove",         &srRemove)   // :file.txt
};
CommandList cmdLSysPhy("cmdSys", "AT+", SIZE_OF_TAB(cmdSysPhy), cmdSysPhy );

/*---------------------------------------------------------------*/
/*                                          */
/*---------------------------------------------------------------*/

// This function has to be added in setup()
int setupTempHumMsg()
{
    serListenerTH.addCmdList(cmdLUserPhy);
    serListenerTH.addCmdList(cmdLSysPhy);

    // I fill info on sketch
    sketchInfo.setFileDateTime(F(__FILE__), F(__DATE__), F(__TIME__));
    // I send identification of sketch
    sendSketchId("");
    sendSketchBuild("");

    return 0;
}


int sendDate(const String& aStr)
{
    tmElements_t tm;
    if ( RTC.read ( tm ) )
    {
      int minutes = tm.Minute ;
      int secondes = tm.Second ;
      String heureString = String ( numeroDix ( tm.Hour ) )
                             + ":"
                             + numeroDix ( minutes )
                             + ":"
                             + numeroDix ( secondes ) ;

      String dateString = String ( tmYearToCalendar ( tm.Year ))
                            + "-"
                            + numeroDix ( tm.Month )
                            + "-"
                            + numeroDix ( tm.Day ) ;

    msgSPrint(getCommand(aStr) + "/OK:"+ dateString +"T"+ heureString);
    }
    return 0;
}

int updateHumCsgn(const String& aStr)
{
    // updateHumCsgn contains cmd and value with this format cmd:value
    // value must exist
    int ind = aStr.indexOf(":");
    if (ind < 0)   {
      msgSError(getCommand(aStr) + F(" cmd needs 1 value"));
      msgSPrint(getCommand(aStr) + "/KO:" +1);
      return 1;
    }

    // we get value part
    String sValue = aStr.substring(ind+1);

    float fValue = sValue.toFloat();
    // toInt will return 0, if it is not an int
    if ( fabs(fValue) < 0.0001 && ( ! sValue.startsWith("0")) )   {
      msgSError(getCommand(aStr) + F(" cmd: value must be float"));
      return 2;
    }
    else if (fValue < 0 || fValue > 100)   {
      msgSError(getCommand(aStr) + F(" cmd: value must be in [0-100]"));
      return 3;
    }

    consigneHum = fValue;

    // I send back OK msg
    msgSPrint(getCommand(aStr) + "/OK:" +fValue);
    // I send back state msg
    //msgSPrint(String(F("state:")) + sValue);

    return 0;
}


int updateTempCsgn(const String& aStr)
{
    // updateHumCsgn contains cmd and value with this format cmd:value
    // value must exist
    int ind = aStr.indexOf(":");
    if (ind < 0)   {
      msgSError(getCommand(aStr) + F(" cmd needs 1 value"));
      msgSPrint(getCommand(aStr) + "/KO:" +1);
      return 1;
    }

    // we get value part
    String sValue = aStr.substring(ind+1);

    float fValue = sValue.toFloat();
    // toInt will return 0, if it is not an int
    if ( fabs(fValue) < 0.0001 && ( ! sValue.startsWith("0")) )   {
      msgSError(getCommand(aStr) + F(" cmd: value must be float"));
      return 2;
    }
    else if (fValue < -10 || fValue > 50)   {
      msgSError(getCommand(aStr) + F(" cmd: value must be in [-10-50]"));
      return 3;
    }

    consigneTemp = fValue;

    // I send back OK msg
    msgSPrint(getCommand(aStr) + "/OK:" +fValue);
    // I send back state msg
    //msgSPrint(String(F("state:")) + sValue);

    return 0;
}
