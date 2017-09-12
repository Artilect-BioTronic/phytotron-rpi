#include "Arduino.h"

#include "TempHumMsg.h"

#include "msgSerial.h"
#include "msgExampleFunction.h"
#include "msg2SDCard.h"

#include <DS1307RTC.h>

SerialListener serListenerTH(SERIAL_MSG);

// list of available commandes (system ctrl) that the arduino will accept
// example:  int sendSketchId(const String& dumb);

// list of available commands (user) that the arduino will accept
Command cmdUserPhy[] = {
    Command("SV",                   &sendFakeVal),
    Command("csgn/humidity/cmd",    &updateHumCsgn,     "i",    "1-90"),
    Command("csgn/temp/cmd",        &updateTempCsgn,    "i",    "-5-50"),
    Command("sendDate",             &sendDate)
};
CommandList cmdLUserPhy("cmdUser", "CM+", SIZE_OF_TAB(cmdUserPhy), cmdUserPhy );

// list of available commands (system ctrl) that the arduino will accept
Command cmdSysPhy[] = {
    Command("idSketch",     &sendSketchId),
    Command("idBuild",      &sendSketchBuild)
};
CommandList cmdLSysPhy("cmdSys", "AT+", SIZE_OF_TAB(cmdSysPhy), cmdSysPhy );

Command cmdSD[] = {
    Command("openStay",     &srStayOpen,    "s,cc", "*,rwascet"),   // :bob.txt,r  filenameDOS8.3 (short names), openMode (read write... )
    Command("open",         &srPreOpen,     "s,cc", "*,rwascet"),   // :prepare to open at each read/write (it is closed immediately after)
    Command("close",        &srClose),          // pas de param
    Command("readln",       &srReadln),         // pas de param
    Command("writeln",      &srWriteln,     "s"),               // :a new lign to write (not \n terminated)
    Command("readNchar",    &srReadNchar,   "i",    "0-200"),   // :nbchar to read in a row
    Command("readNln",      &srReadNln,     "i",    "0-200"),   // :nb lines to read in a row
    Command("move",         &srMove,        "s"),       // :str2search
    Command("dump2",        &srDump2),                  // pas de param
    Command("ls",           &srLs,          "cc",   "rsda"),      // :sr  (recurse size ...)
    Command("rename",       &srRename,      "s,s"),     // :/adir/old,new
    Command("mkdir",        &srMkdir,       "s"),       // :/adir
    Command("rm",           &srRemove,      "s")        // :file.txt
};
CommandList cmdLSD("cmdSD", "SD+", SIZE_OF_TAB(cmdSD), cmdSD );

/*---------------------------------------------------------------*/
/*                                          */
/*---------------------------------------------------------------*/

// This function has to be added in setup()
int setupTempHumMsg()
{
    serListenerTH.addCmdList(cmdLUserPhy);
    serListenerTH.addCmdList(cmdLSysPhy);
    serListenerTH.addCmdList(cmdLSD);

    // following line is copied in main sketch.ino file
    // I fill info on sketch
//    sketchInfo.setFileDateTime(F(__FILE__), F(__DATE__), F(__TIME__));
    // I send identification of sketch
    cmdLSysPhy.readInternalMessage(F("idSketch"));
    cmdLSysPhy.readInternalMessage(F("idBuild"));

    return 0;
}


int sendDate(const CommandList& aCL, Command &aCmd, const String& aInput)
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

    aCL.msgPrint(aCL.getCommand(aInput) + "/state:"+ dateString +"T"+ heureString);
    aCL.msgOK(aInput, dateString +"T"+ heureString);
    }
    return 0;
}

// "i",  "0-90"
int updateHumCsgn(const CommandList& aCL, Command &aCmd, const String& aInput)
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


    consigneHum = iValue;

    ecritConsigneDansFichier();

    // I send back state msg
    sendConsigne();
    // I send back OK msg
    aCL.msgOK(aInput, String(iValue));

    return 0;
}

// "i",  "-5-50"
int updateTempCsgn(const CommandList& aCL, Command &aCmd, const String& aInput)
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


    consigneTemp = iValue;

    ecritConsigneDansFichier();

    // I send back state msg
    sendConsigne();
    // I send back OK msg
    aCL.msgOK(aInput, String(iValue));

    return 0;
}


/*---------------------------------------------------------------*/
/*       fonctions de remplacement materiel                      */
/*---------------------------------------------------------------*/

void fakeReleveValeurs()
{
    temperatureInterieureEntiere = 25 ;
    humiditeInterieureEntiere = 41 ;
    temperatureExterieureEntiere = 25.5 ;
    humiditeExterieureEntiere = 42 ;
}
