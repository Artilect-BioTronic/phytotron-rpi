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

Cmd2File::Cmd2File(): file_(NULL),fileName_(""),isOpened_(false),isPreOpened_(false),filePos_(0)  {}
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

int Cmd2File::stayOpen(const String& aFileName, uint8_t aMode)
{
    if ( (aFileName.length() <= 0) || (aFileName.length() > 12) )
        return -1;

    // if a file was open  we close it
    //   if it was preOpened it was already closed
    if (isOpened_)
        close();

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
        *file_ = SD.open(aFileName, aMode);
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

int Cmd2File::preOpen(const String& aFileName, uint8_t aMode)
{
    if ( (aFileName.length() <= 0) || (aFileName.length() > 12) )
        return -1;

    // if a file was open  we close it
    //   if it was preOpened it was already closed
    if (isOpened_)
        close();

    file_ = new FILELIKE();
    *file_ = SD.open(aFileName, aMode);
    if (! (*file_) )   {
        free (file_);
        file_ = NULL;
        return -1;
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

    if (! file_->open(fileName_.c_str(), O_WRITE | O_READ))
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

// print ls result through aStream, with option aString
int Cmd2File::ls(Stream &aStream, const uint8_t aOption)
{
    if (aOption < 0  ||  15 < aOption)
        return -1;

    SD.ls(&aStream, "/", aOption);
    return 0;
}

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


int srStayOpen(const String& argFile)
{
    // arg must exist after :
    int ind = argFile.indexOf(":");
    if (ind < 0)   {
        msgSError(getCommand(argFile) + F(":cmd needs 2 values"));
        msgSPrintln(getCommand(argFile) + F("/KO:") + 1);
        return 1;
    }

    // we get all args
    String sValueAll = argFile.substring(ind+1);

    // 2 arg separated by ,
    ind = sValueAll.indexOf(",");
    if (ind < 1)   {
        msgSError(getCommand(argFile) + F(":cmd needs 2 values"));
        msgSPrintln(getCommand(argFile) + F("/KO:") + 10);
        return 10;
    }

    // we get 1st value part
    String sFile = sValueAll.substring(0,ind);

    // we get 1st value part
    int iMode = sValueAll.substring(ind+1).toInt();

    // mode values are in [1..127]
    // open modes are (0X40 to 0X01): O_TRUNC | O_EXCL | O_CREAT | O_SYNC | O_APPEND | O_WRITE | O_READ
    if ( iMode <= 0  || iMode > 127 )   {
        msgSError(getCommand(argFile) + F(":mode must be in [1..127]"));
        msgSPrintln(getCommand(argFile) + F("/KO:2"));
        return 2;
    }

    int cr = cmd2File.stayOpen(sFile,iMode);

    if (cr == 0)   {
        // I send back state msg
        msgSPrintln(getCommand(argFile) + F("/OK:") + cr);
    }
    else   {
        msgSPrintln(getCommand(argFile) + F("/KO:") + cr);
    }
    return 0;
}

int srPreOpen(const String& argFile)
{
    // arg must exist after :
    int ind = argFile.indexOf(":");
    if (ind < 0)   {
        msgSError(getCommand(argFile) + F(":cmd needs 2 values"));
        msgSPrintln(getCommand(argFile) + F("/KO:1"));
        return 1;
    }

    // we get all args
    String sValueAll = argFile.substring(ind+1);

    // 2 arg separated by ,
    ind = sValueAll.indexOf(",");
    if (ind < 1)   {
        msgSError(getCommand(argFile) + F(":cmd needs 2 values"));
        msgSPrintln(getCommand(argFile) + F("/KO:10"));
        return 10;
    }

    // we get 1st value part
    String sFile = sValueAll.substring(0,ind);

    // we get 1st value part
    int iMode = sValueAll.substring(ind+1).toInt();

    // mode values are in [1..127]
    // open modes are (0X40 to 0X01): O_TRUNC | O_EXCL | O_CREAT | O_SYNC | O_APPEND | O_WRITE | O_READ
    if ( iMode <= 0  || iMode > 127 )   {
        msgSError(getCommand(argFile) + F(":mode must be in [1..127]"));
        msgSPrintln(getCommand(argFile) + F("/KO:2"));
      return 2;
    }

    int cr = cmd2File.preOpen(sFile,iMode);

    if (cr == 0)   {
        // I send back state msg
        msgSPrintln(getCommand(argFile) + F("/OK:") + cr);
    }
    else   {
        msgSPrintln(getCommand(argFile) + F("/KO:") + cr);
    }
    return 0;
}

int srClose(const String& argFile)
{
    // close file
    int cr = cmd2File.close();

    // I send back state msg
    msgSPrintln(getCommand(argFile) + F("/OK:") + cr);

    return 0;
}

int srReadln(const String& dumb)
{
    String readString="";

    // the file is read until \n char, string goes in readString
    // returns the number of read char,  0 if none, -1 if error
    int cr = cmd2File.readln(readString);

    if (cr <= 0)
        msgSPrintln(getCommand(dumb) + F("/KO:") + cr);
    else
        // I send back the  String  that was read
        msgSPrintln(getCommand(dumb) + F("/OK:") + readString);

    return 0;
}

int srWriteln(const String& aToWrite)
{
    // arg must exist after :
    int ind = aToWrite.indexOf(":");
    if (ind < 0)   {
        msgSError(getCommand(aToWrite) + F(":cmd needs 1 value"));
        msgSPrintln(getCommand(aToWrite) + F("/KO:1"));
        return 1;
    }

    // we get all args
    String sValue = aToWrite.substring(ind+1);

    // returns the number of printed char,  0 if none, -1 if error
    int cr = cmd2File.writeln(sValue);

    if (cr < 0)
        msgSPrintln(getCommand(aToWrite) + F("/KO:") +cr);
    else
        // I send back OK
        msgSPrintln(getCommand(aToWrite) + F("/OK:") +cr);

    return 0;
}

int srMove(const String& a2Search)
{
    // arg must exist after :
    int ind = a2Search.indexOf(":");
    if (ind < 0)   {
        msgSError(getCommand(a2Search) + F(":cmd needs 1 value"));
        msgSPrintln(getCommand(a2Search) + F("/KO:1"));
        return 1;
    }

    // we get args
    String sValue = a2Search.substring(ind+1);

    int cr = cmd2File.moveTo(sValue);

    if (cr >= 0)
        msgSPrintln(getCommand(a2Search) + F("/OK"));
    else
        msgSPrintln(getCommand(a2Search) + F("/KO:") +cr);

    return 0;
}

int srReadNchar(const String& aString)
{
    // arg must exist after :
    int ind = aString.indexOf(":");
    if (ind < 0)   {
        msgSError(getCommand(aString) + F(":cmd needs 1 value"));
        msgSPrintln(getCommand(aString) + F("/KO:-1"));
        return -1;
    }

    // we get args
    int nbChar = aString.substring(ind+1).toInt();
    if ((nbChar <=0) || (200 < nbChar))   {
        msgSPrintln(getCommand(aString) + F("/KO:nbChar must be in [0-200]"));
        return -2;
    }

    String strRead="";
    if (! strRead.reserve(nbChar))   {
        msgSPrintln(getCommand(aString) + F("/KO:-3"));
        return -3;
    }
    int cr = cmd2File.readNchar(strRead, nbChar);

    if (cr > 0)
        msgSPrintln(String("+") +cr +","+ getCommand(aString) + F("/OK:") + strRead);
    else
        msgSPrintln(getCommand(aString) + F("/KO:") + cr);

    return 0;
}

int srRename(const String& aString)
{
    // arg must exist after :
    int ind = aString.indexOf(":");
    if (ind < 0)   {
        msgSError(getCommand(aString) + F(":cmd needs 2 values"));
        msgSPrintln(getCommand(aString) + F("/KO:-1"));
        return -1;
    }

    // we get all args
    String sValueAll = aString.substring(ind+1);

    // 2 arg separated by ,
    ind = sValueAll.indexOf(",");
    if (ind < 1)   {
        msgSError(getCommand(aString) + F(":cmd needs 2 values"));
        msgSPrintln(getCommand(aString) + F("/KO:-10"));
        return -10;
    }

    // we get 1st value part
    String sOld = sValueAll.substring(0,ind);

    // we get 1st value part
    String sNew = sValueAll.substring(ind+1);


    int cr = cmd2File.rename(sOld, sNew);

    if (cr == 0)
        msgSPrintln(getCommand(aString) + F("/OK"));
    else
        msgSPrintln(getCommand(aString) + F("/KO:") + cr);

    return 0;
}

int srRemove(const String& aString)
{
    // arg must exist after :
    int ind = aString.indexOf(":");
    if (ind < 0)   {
        msgSError(getCommand(aString) + F(":cmd needs 1 value"));
        msgSPrintln(getCommand(aString) + F("/KO:-1"));
        return -1;
    }

    // we get args
    String sValue = aString.substring(ind+1);

    int cr = cmd2File.remove(sValue);

    if (cr == 0)
        msgSPrintln(getCommand(aString) + F("/OK"));
    else
        msgSPrintln(getCommand(aString) + F("/KO:") + cr);

    return 0;
}

int srLs(const String& aString)
{
    // arg must exist after :
    int ind = aString.indexOf(":");
    if (ind < 0)   {
        msgSError(getCommand(aString) + F(":cmd needs 1 value"));
        msgSPrintln(getCommand(aString) + F("/KO:-1"));
        return -1;
    }

    // we get args
    int iMode = aString.substring(ind+1).toInt();

    // mode values are in [1..15]
    // modes are (0X08 to 0X01): LS_R | LS_SIZE | LS_DATE | LS_A
    if ( iMode <= 0  || iMode > 15 )   {
        msgSError(getCommand(aString) + F(":mode must be in [1..15]"));
        msgSPrintln(getCommand(aString) + F("/KO:-2"));
      return -2;
    }

    int cr = cmd2File.ls(SERIAL_MSG, iMode);

    if (cr == 0)
        msgSPrintln(getCommand(aString) + F("/OK"));
    else
        msgSPrintln(getCommand(aString) + F("/KO:") + cr);

    return 0;
}

int srMkdir(const String& aString)
{
    // arg must exist after :
    int ind = aString.indexOf(":");
    if (ind < 0)   {
        msgSError(getCommand(aString) + F(":cmd needs 1 value"));
        msgSPrintln(getCommand(aString) + F("/KO:-1"));
        return -1;
    }

    // we get args
    String sValue = aString.substring(ind+1);

    int cr = cmd2File.mkdir(sValue);

    if (cr == 0)
        msgSPrintln(getCommand(aString) + F("/OK"));
    else
        msgSPrintln(getCommand(aString) + F("/KO:") + cr);

    return 0;
}

int srDump2(const String& aString)
{
    dump2("COM.CSV");

    msgSPrintln(getCommand(aString) + F("/OK"));

    return 0;
}

