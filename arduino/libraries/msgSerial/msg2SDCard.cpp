#include "msg2SDCard.h"

// The standard SD library is replaced by SdFat library that is more complete:
//   https://github.com/greiman/SdFat
//#include "SD.h"
#include "SdFat.h"
SdFat SD;

#include "msgSerial.h"


Cmd2File cmd2File;

const char Cmd2File::MOVE_BEGIN[] = "BEGIN";
const char Cmd2File::MOVE_END[]   = "END";
const char Cmd2File::endOfLine_[]   = "\n";  // could be "\r\n" for windows

Cmd2File::Cmd2File(): file_(NULL),fileName_(""),
    isOpened_(false),isPreOpened_(false),iMode_(O_READ),
    filePos_(0)  {}
Cmd2File::~Cmd2File()   {   close();  }

int Cmd2File::close()   {
    if ( (file_ != NULL) && isOpened_ )   {
        file_->close();
        free (file_);
    }
    file_ = NULL;
    isOpened_ = false;
    isPreOpened_ = false;
    filePos_ = 0;
    return 0;
}

int Cmd2File::stayOpen(const String& aFileName, const String& aMode)
{
    // DOS format limits filename to  8.3 characters
    if ( (aFileName.length() <= 0) || (aFileName.length() > 12) )
        return -1;

    // if a file was open  we close it
    //   if it was preOpened it was already closed
    if (isOpened_)
        close();

    // converts string aMode, from letters tecsawr to constants
    // O_TRUNC | O_EXCL | O_CREAT | O_SYNC | O_APPEND | O_WRITE | O_READ
    uint8_t iMode_=0;
    for (int i=0; i<aMode.length(); i++)   {
        switch(aMode.charAt(i))
        {
        case('r') :
            iMode_ += O_READ;
            break;
        case('w') :
            iMode_ += O_WRITE;
            break;
        case('a') :
            iMode_ += O_APPEND;
            break;
        case('s') :
            iMode_ += O_SYNC;
            break;
        case('c') :
            iMode_ += O_CREAT;
            break;
        case('e') :
            iMode_ += O_EXCL;
            break;
        case('t') :
            iMode_ += O_TRUNC;
            break;
        default :
            return -2 ;     // unknown letter not permitted
        }
    }

    // if the name is fake, we attach to a fake file
    if (aFileName.equals("fake"))   {
        fileName_ = aFileName;
//        file_ = &fakeFile;
        isOpened_ = true;
//        return fakeFile.open(aFileName);
    }
    else   // it is supposed to be a SD file
    {
        file_ = new FILELIKE();
        *file_ = SD.open(aFileName, iMode_);
        if (! (*file_) )   {
            free (file_);
            file_ = NULL;
            return -1;
        }

        fileName_ = aFileName;
        isOpened_ = true;
        return 0;
    }
    return 0;
}

int Cmd2File::preOpen(const String& aFileName, const String& aMode)
{
    if ( ! isPathCompatibleDOS83(aFileName) )
        return -1;

    // converts string aMode, from letters tecsawr to constants
    // O_TRUNC | O_EXCL | O_CREAT | O_SYNC | O_APPEND | O_WRITE | O_READ
    boolean isTrunc = false;
    iMode_=0;
    for (int i=0; i<aMode.length(); i++)   {
        switch(aMode.charAt(i))
        {
        case('r') :
            iMode_ += O_READ;
            break;
        case('w') :
            iMode_ += O_WRITE;
            break;
        case('a') :
            iMode_ += O_APPEND;
            break;
        case('s') :
            iMode_ += O_SYNC;
            break;
        case('c') :
            iMode_ += O_CREAT;
            break;
        case('e') :
            iMode_ += O_EXCL;
            break;
        case('t') :
            isTrunc = true;
            break;
        default :
            return -2 ;     // unknown letter not permitted
        }
    }

    // if a file was open  we close it
    //   if it was preOpened it was already closed
    if (isOpened_)
        close();

    // if O_TRUNC is in open mode, it is not memorized for each future open
    // we open once with O_TRUNC, and close it immeidately
    if (isTrunc)   {
        file_ = new FILELIKE();
        *file_ = SD.open(aFileName, iMode_+O_TRUNC);
        if (! (*file_) )   {
            free (file_);
            file_ = NULL;
            return -3;
        }
        else   {
            file_->close();
            free (file_);
            file_ = NULL;
        }
    }

    file_ = new FILELIKE();
    *file_ = SD.open(aFileName, iMode_);
    if (! (*file_) )   {
        free (file_);
        file_ = NULL;
        return -3;
    }
    else   {
        filePos_ = file_->curPosition();  // end of file if open with O_APPEND
        file_->close();
    }

    fileName_ = aFileName;
    isPreOpened_ = true;
    return 0;
}

int Cmd2File::tmpOpen()
{
    if (! isPreOpened_)
        return -1;

    if (! file_->open(fileName_.c_str(), iMode_))
        return -2;
    if (! file_->seek(filePos_))
        return -3;

    return 0;
}

int Cmd2File::tmpClose()
{
    if (! isPreOpened_)
        return -1;

    filePos_ = file_->curPosition();
    file_->close();
    return 0;
}

void dump2(String aFileName){
    File dataFile = SD.open(aFileName);

    // if the file is available, write to it:
    if (dataFile) {
        while (dataFile.available()) {
            Serial.write(dataFile.read());
        }
        dataFile.close();
    }
    // if the file isn't open, pop up an error:
    else {
        Serial.println(String("error opening ") + aFileName);
    }
}

// the file is read until \n char, string goes in readString
// returns the number of read char,  0 if none, -1 if error
// it open / close file if needed and call to readlnNoOpen for reading
int Cmd2File::readln(String& aString)
{
    aString = "";
    if ( (! isOpened_) && (! isPreOpened_) )
        return -1;

    if (isPreOpened_)  { // we open  before each access
        int cr = tmpOpen();
        if (cr != 0)
            return cr-10;
    }

    int cr = readlnNoOpen(aString);

    if (isPreOpened_)   // we close  after each access
        tmpClose();

    if (cr <=0)
        return cr;
    else
        return aString.length();
}

// the file is read until \n char, string goes in readString
// returns the number of read char,  0 if none, -1 if error
int Cmd2File::readlnNoOpen(String& aString)
{
    aString = "";
    if ( (! isOpened_) && (! isPreOpened_) )
        return -1;

    int newChar = 0;
    // read until \n
    while ( (file_->available()) && (newChar >= 0) && (newChar < 256) && (newChar != '\n') )   {
        newChar = file_->read();
        if ( (newChar >= 0) && (newChar < 256) )
            aString += char(newChar);
    }

    return aString.length();
}

// print aString  +  '\n'
// returns the number of printed char,  0 if none, -1 if error
int Cmd2File::writeln(const String& aString)
{
    if ( (! isOpened_) && (! isPreOpened_) )
        return -1;

    if (isPreOpened_)  { // we open  before each access
        int cr = tmpOpen();
        if (cr != 0)
            return cr-10;
    }

    int cr = file_->print(aString);

    if (cr >= 0)   // if no pb, we end with newline char
        cr = file_->print(endOfLine_);

    if (isPreOpened_)   // we close  after each access
        tmpClose();

    return cr;
}

// reads nbChar  char  and put it in strBuf
// it returns the nb of read char on success,  negative otherwise
int Cmd2File::readNchar(String& strBuf, int nbChar)   {
    if ( (! isOpened_) && (! isPreOpened_) )
        return -1;

    if (isPreOpened_)  { // we open  before each access
        int cr = tmpOpen();
        if (cr != 0)
            return cr-10;
    }

    int cr = readNcharNoOpen(strBuf, nbChar);

    if (isPreOpened_)   // we close  after each access
        tmpClose();

    // if cr >0 , cr  is the nb of read char
    return cr;
}

// reads nbChar  char  and put it in strBuf
// it returns the nb of read char on success,  negative otherwise
// the file must be open; it will not open/close contrary to readNchar
int Cmd2File::readNcharNoOpen(String& strBuf, int nbChar)   {
    if ( (! isOpened_) && (! isPreOpened_) )
        return -1;

    strBuf = "";
    int cr=0;
    int nbRead=0;
    for (nbRead=0; nbRead<nbChar && cr==0; nbRead++)   {
        int newChar = file_->read();
        if ( (newChar >= 0) && (newChar < 256) )
            strBuf += char(newChar);
        else
            cr = -3;
    }
    if (cr == -3)
        nbRead--;

    // if cr==0 or cr<0,  nbRead  is the nb of read char
    return nbRead;
}

// it reads file until it finds aString
// it manages open / close  and call to moveToNoOpen
long Cmd2File::moveTo(const String& aString)   {
    if ( (! isOpened_) && (! isPreOpened_) )
        return -1;

    if (isPreOpened_)  { // we open  before each access
        long cr = tmpOpen();
        if (cr != 0)
            return cr-10;
    }

    long cr = moveToNoOpen(aString);

    if (isPreOpened_)   // we close  after each access
        tmpClose();

    return cr;
}

// it reads file until it finds aString
// it can go to BEGIN or END of file
// it returns 0 on success, -1 or negative if it fails
// the file must be open; it will not open/close contrary to moveTo
long Cmd2File::moveToNoOpen(const String& aString)
{
    if ( (! isOpened_) && (! isPreOpened_) )
        return -1;
    if ( (aString.length() <= 0) || (aString.length() > 100) )
        return -2;

    // I can use 2 special keywords
    if (aString.equalsIgnoreCase(MOVE_BEGIN))   {
        return file_->seek(0);
    }
    if (aString.equalsIgnoreCase(MOVE_END))   {
        return file_->seek(file_->size());
    }

    // We memorize position before search
    long posInitial = file_->position();

    // file is read, until the next occurrence of aString
    boolean isStrFound = false;
    boolean isReadOK = true;
    char char1 = aString.charAt(0);
    int iCurChar = 0;
    while ( (! isStrFound) && isReadOK )   {
        // first, we check if 1st char is found
        if (file_->available())
            iCurChar = file_->read();
        else
            isReadOK = false;
        isReadOK = isReadOK && (iCurChar >= 0) && (iCurChar < 256);

        // if 1st char OK, we will read full length strBuf,  we will check if strBuf == aString
        long lastPos = file_->position();  // we will go back to lastPos on failure
        String strBuf="";
        if (isReadOK && (iCurChar == char1))   {
            file_->seek(lastPos-1);   // rewind
            int cr = readNcharNoOpen(strBuf, aString.length());  // read full length string
            if (cr < 0)   {
                file_->seek(posInitial);   // failure, we get back to init position
                return -1;
            }

            /*****     if aString is found   *****/
            if (aString.equals(strBuf))   {
                file_->seek(lastPos-1);     // we get back to begin of found string
                return lastPos-1;           // return position in file
            }
            else
                file_->seek(lastPos);   // rewind
        }
    }

    // if aString could not be found, we get back to init position
    file_->seek(posInitial);

    return posInitial;
}

//------------------------------------------------------------------------------
// re-write of FatFile::ls
//   because I want to frame output of ls between  getCmdPrefix  and   getCmdEnd
void FatFile_ls(const CommandList& aCL, FatFile& aDir, uint8_t flags, uint8_t indent) {
    FatFile file;
    print_t* pr = aCL.getStream();
    aDir.rewind();
    while (file.openNext(&aDir, O_READ)) {
        // indent for dir level
        if (!file.isHidden() || (flags & LS_A)) {
            pr->print(aCL.getCmdPrefix());
            pr->print(F("ls:"));

            for (uint8_t i = 0; i < indent; i++) {
                pr->write(' ');
            }
            if (flags & LS_DATE) {
                file.printModifyDateTime(pr);
                pr->write(' ');
            }
            if (flags & LS_SIZE) {
                file.printFileSize(pr);
                pr->write(' ');
            }
            file.printName(pr);
            if (file.isDir()) {
                pr->write('/');
            }
//            pr->write('\r');
//            pr->write('\n');
            pr->print(aCL.getCmdEnd());

            if ((flags & LS_R) && file.isDir()) {
                FatFile_ls(aCL, file, flags, indent + 2);
            }
        }
        file.close();
    }
}

// print ls result through aStream, with option aString
int Cmd2File::ls(const CommandList& aCL, const String& aMode)
{
    // converts string aMode, from letters rsda to constants
    // (0X08 to 0X01): LS_R | LS_SIZE | LS_DATE | LS_A
    int iMode=0;
    for (int i=0; i<aMode.length(); i++)   {
        switch(aMode.charAt(i))
        {
        case('r') :
            iMode += LS_R;
            break;
        case('s') :
            iMode += LS_SIZE;
            break;
        case('d') :
            iMode += LS_DATE;
            break;
        case('a') :
            iMode += LS_A;
            break;
        default :
            return -10 ;     // unknown letter not permitted
        }
    }

//    SD.ls(&aStream, "/", iMode);
    FatFile dir;
    dir.open(SD.vwd(), "/", O_READ);
    FatFile_ls(aCL, dir, iMode, 0);

    return 0;
}

//void ls(print_t* pr, const char* path, uint8_t flags) {
//  FatFile dir;
//  dir.open(vwd(), path, O_READ);
//  dir.ls(pr, flags);
//}

//
// it returns 0 on success, -1 or negative if it fails
int Cmd2File::rename(const String& aOld, const String &aNew)
{
    if ( (aOld.length() <= 0) || (aOld.length() > 100) )
        return -1;
    if ( (aNew.length() <= 0) || (aNew.length() > 100) )
        return -2;

    if (SD.rename(aOld.c_str(), aNew.c_str()))
        return 0;
    else
        return -3;
}

//
int Cmd2File::remove(const String& aString)
{
    if ( (aString.length() <= 0) || (aString.length() > 100) )
        return -1;

    boolean cr=false;
    if (aString.endsWith("/"))   // remove a dir
        cr = SD.rmdir(aString.c_str());
    else   //  remove a file
        cr = SD.remove(aString.c_str());

    if ( cr )
        return 0;
    else
        return -2;
}

//
int Cmd2File::mkdir(const String& aString)
{
    if ( (aString.length() <= 0) || (aString.length() > 100) )
        return -1;

    if (SD.mkdir(aString.c_str(), true))
        return 0;
    else
        return -2;
}

/*---------------------------------------------------------------*/
/*          callback function to put in CommanList               */
/*---------------------------------------------------------------*/

// "s,ss",  "*,rwascet"   ->  read write append size create excl trunc
// open modes are transcripted as (0X40 to 0X01): O_TRUNC | O_EXCL | O_CREAT | O_SYNC | O_APPEND | O_WRITE | O_READ
int srStayOpen(const CommandList& aCL, Command &aCmd, const String& aInput)
{
    ParsedCommand parsedCmd(aCL, aCmd, aInput);

    // verify that msg with arguments is OK with format and limit
    if (parsedCmd.verifyFormatMsg(aCmd, aInput) != ParsedCommand::NO_ERROR)
        return aCL.returnKO(aCmd, parsedCmd);

    // get values
    String sFile = parsedCmd.getValueStr(1);
    String sMode = parsedCmd.getValueStr(2);

    // we check that parsedCmd has not detected any error
    if (parsedCmd.hasError())
        return aCL.returnKO(aCmd, parsedCmd);

    // DOS format limits filename to  8.3 characters
    if ( ! isPathCompatibleDOS83(sFile) )   {
         aCL.msgError(String(F("file is limited to format 8.3: ")) + sFile);
         aCL.msgKO(aInput, -5);
         return 0;
    }

    int cr = cmd2File.stayOpen(sFile,sMode);

    if (cr == 0)
        aCL.msgOK(aInput, cr);
    else
        aCL.msgKO(aInput, cr);

    return 0;
}

boolean isPathCompatibleDOS83(const String& aFile)
{
    if (aFile.length() <= 0)   return false;

    // get basename of  aFile
    String basename = aFile;
    int indSlash = aFile.lastIndexOf("/");
    if (indSlash >= 0)
        basename = aFile.substring(indSlash+1);

    // split  name.ext  in  name  .  ext
    String name = basename;
    String ext = "";        // ok if there is no ".ext"
    int indDot = basename.indexOf(".");
    // if there is  .ext
    if (indDot >= 0)   {
        name = basename.substring(0, indDot);
        ext = basename.substring(indDot+1);
    }

    // basename is <= 8   ,  ext is <= 3
    if ( (name.length() <= 8) && (ext.length() <= 3) )
        return true;
    else
        return false;
}

// "s,ss",  "*,rwascet"   ->  read write append size create excl trunc
// open modes are transcripted as (0X40 to 0X01): O_TRUNC | O_EXCL | O_CREAT | O_SYNC | O_APPEND | O_WRITE | O_READ
int srPreOpen(const CommandList& aCL, Command &aCmd, const String& aInput)
{
    ParsedCommand parsedCmd(aCL, aCmd, aInput);

    // verify that msg with arguments is OK with format and limit
    if (parsedCmd.verifyFormatMsg(aCmd, aInput) != ParsedCommand::NO_ERROR)
        return aCL.returnKO(aCmd, parsedCmd);

    // get values
    String sFile = parsedCmd.getValueStr(1);
    String sMode = parsedCmd.getValueStr(2);

    // we check that parsedCmd has not detected any error
    if (parsedCmd.hasError())
        return aCL.returnKO(aCmd, parsedCmd);

    // DOS format limits filename to  8.3 characters
    if ( ! isPathCompatibleDOS83(sFile) )   {
         aCL.msgError(String(F("file is limited to format 8.3: ")) + sFile);
         aCL.msgKO(aInput, -5);
         return 0;
    }

    int cr = cmd2File.preOpen(sFile,sMode);

    if (cr == 0)
        aCL.msgOK(aInput, cr);
    else
        aCL.msgKO(aInput, cr);

    return 0;
}

// "", ""
int srClose(const CommandList& aCL, Command &aCmd, const String& aInput)
{
    // close file
    int cr = cmd2File.close();

    // I send back state msg
    aCL.msgOK(aInput, cr);

    return 0;
}

// "", ""
int srReadln(const CommandList& aCL, Command &aCmd, const String& aInput)
{
    String readString="";

    // the file is read until \n char, string goes in readString
    // returns the number of read char,  0 if none, -1 if error
    int cr = cmd2File.readln(readString);
    // here we get rid of ending \n
    readString.remove(readString.length()-1);

    if (cr > 0)
        aCL.msgOK(aInput, readString);
    else
        aCL.msgKO(aInput, cr);

    return 0;
}

// "s", ""
int srWriteln(const CommandList& aCL, Command &aCmd, const String& aInput)
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


    // returns the number of printed char,  0 if none, -1 if error
    int cr = cmd2File.writeln(sValue);

    if (cr >= 0)
        aCL.msgOK(aInput, cr);
    else
        aCL.msgKO(aInput, cr);

    return 0;
}

// "s", ""
int srMove(const CommandList& aCL, Command &aCmd, const String& aInput)
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


    int cr = cmd2File.moveTo(sValue);

    if (cr >= 0)
        aCL.msgOK(aInput, "");
    else
        aCL.msgKO(aInput, cr);

    return 0;
}

// "d", "0-200"
int srReadNchar(const CommandList& aCL, Command &aCmd, const String& aInput)
{
    ParsedCommand parsedCmd(aCL, aCmd, aInput);

    // verify that msg with arguments is OK with format and limit
    if (parsedCmd.verifyFormatMsg(aCmd, aInput) != ParsedCommand::NO_ERROR)
        return aCL.returnKO(aCmd, parsedCmd);

    // get values
    int nbChar = parsedCmd.getValueInt(1);

    // we check that parsedCmd has not detected any error
    if (parsedCmd.hasError())
        return aCL.returnKO(aCmd, parsedCmd);


    String strRead="";
    if (! strRead.reserve(nbChar))   {
        aCL.msgKO(aInput, -3);
        return -3;
    }
    int cr = cmd2File.readNchar(strRead, nbChar);

    if (cr > 0)
        aCL.msgPrint(String("+") +cr +","+ aCL.getCommand(aInput) + F("/OK:") + strRead);
    else
        aCL.msgKO(aInput, cr);

    return 0;
}

// "d", "0-200"
int srReadNln(const CommandList& aCL, Command &aCmd, const String& aInput)
{
    ParsedCommand parsedCmd(aCL, aCmd, aInput);

    // verify that msg with arguments is OK with format and limit
    if (parsedCmd.verifyFormatMsg(aCmd, aInput) != ParsedCommand::NO_ERROR)
        return aCL.returnKO(aCmd, parsedCmd);

    // get values
    int nbLine = parsedCmd.getValueInt(1);

    // we check that parsedCmd has not detected any error
    if (parsedCmd.hasError())
        return aCL.returnKO(aCmd, parsedCmd);


    String strRead="";
    // I test that I have some memory available
    if (! strRead.reserve(200))   {
        aCL.msgKO(aInput, -3);
        return -3;
    }
    int cr = 0, i = 0;
    for (i=1; i <= nbLine; i++)   {
        // returns the number of read char,  0 if none, -1 if error
        cr = cmd2File.readln(strRead);
        if (cr <= 0)    break;
        // here we get rid of ending \n
        strRead.remove(strRead.length()-1);
        aCL.msgPrint(aCL.getCommand(aInput) + "/" + i + ":" + strRead);
    }


    if (cr >= 0 && i>=2)
        aCL.msgOK(aInput, String(i-1) );
    else
        aCL.msgKO(aInput, cr);

    return 0;
}

// "s,s", ""
int srRename(const CommandList& aCL, Command &aCmd, const String& aInput)
{
    ParsedCommand parsedCmd(aCL, aCmd, aInput);

    // verify that msg with arguments is OK with format and limit
    if (parsedCmd.verifyFormatMsg(aCmd, aInput) != ParsedCommand::NO_ERROR)
        return aCL.returnKO(aCmd, parsedCmd);

    // get values
    String sOld = parsedCmd.getValueStr(1);
    String sNew = parsedCmd.getValueStr(2);

    // we check that parsedCmd has not detected any error
    if (parsedCmd.hasError())
        return aCL.returnKO(aCmd, parsedCmd);


    int cr = cmd2File.rename(sOld, sNew);

    if (cr == 0)
        aCL.msgOK(aInput, "");
    else
        aCL.msgKO(aInput, cr);

    return 0;
}

// "s", "*"
int srRemove(const CommandList& aCL, Command &aCmd, const String& aInput)
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


    int cr = cmd2File.remove(sValue);

    if (cr == 0)
        aCL.msgOK(aInput, "");
    else
        aCL.msgKO(aInput, cr);

    return 0;
}

// "ss", "rsda"   -> recurse size date all
int srLs(const CommandList& aCL, Command &aCmd, const String& aInput)
{
    ParsedCommand parsedCmd(aCL, aCmd, aInput);

    // verify that msg with arguments is OK with format and limit
    if (parsedCmd.verifyFormatMsg(aCmd, aInput) != ParsedCommand::NO_ERROR)
        return aCL.returnKO(aCmd, parsedCmd);

    // get values
    String sMode = parsedCmd.getValueStr(1);

    // we check that parsedCmd has not detected any error
    if (parsedCmd.hasError())
        return aCL.returnKO(aCmd, parsedCmd);


//    int cr = cmd2File.ls(*(aCL.getStream()), sMode);
    int cr = cmd2File.ls(aCL, sMode);

    if (cr == 0)
        aCL.msgOK(aInput, "");
    else
        aCL.msgKO(aInput, cr);

    return 0;
}

// "s", "*"
int srMkdir(const CommandList& aCL, Command &aCmd, const String& aInput)
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


    int cr = cmd2File.mkdir(sValue);

    if (cr == 0)
        aCL.msgOK(aInput, "");
    else
        aCL.msgKO(aInput, cr);

    return 0;
}

int srDump2(const CommandList& aCL, Command &aCmd, const String& aInput)
{
    dump2("COM.CSV");

    aCL.msgOK(aInput, "");

    return 0;
}

