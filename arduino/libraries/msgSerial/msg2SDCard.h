#include "Arduino.h"

// The standard SD library is replaced by SdFat library that is more complete:
//   https://github.com/greiman/SdFat
//#include "SD.h"
#include "SdFat.h"
extern SdFat SD;

#include "msgSerial.h"

/*---------------------------------------------------------------*/
/*                                          */
/*---------------------------------------------------------------*/

//#define FILELIKE  SDLib::File
#define FILELIKE  File

class Cmd2File
{
private:
    FILELIKE * file_;
    String fileName_;
    boolean isOpened_;
    boolean isPreOpened_;
    long filePos_;

public:
    static const char MOVE_BEGIN[];
    static const char MOVE_END[];
    static const char endOfLine_[];

public:
    int stayOpen(const String& aFileName, uint8_t aMode);
    int preOpen(const String& aFileName, uint8_t aMode);
    int close();
    int tmpOpen();
    int tmpClose();
    int readNchar(String& strBuf, int nbChar);
    int readNcharNoOpen(String& strBuf, int nbChar);
    int readln(String& aString);
    int writeln(const String& aString);
    int moveTo(const String& aString);
    int moveToNoOpen(const String& aString);
    int rename(const String& aOld, const String& aNew);
    int remove(const String& aString);
    int mkdir(const String& aString);
    int ls(Stream &aStream, const uint8_t aOption);
    Cmd2File();
    ~Cmd2File();
};


int srStayOpen(const String& argFile);

int srPreOpen(const String& argFile);

int srClose(const String& dumb);

int srReadln(const String& dumb);

int srWriteln(const String& aToWrite);

int srMove(const String& a2Search);

int srReadNchar(const String& aString);

int srRename(const String& aString);

int srRemove(const String& aFilename);

int srLs(const String& arg);

int srMkdir(const String& aString);

int srDump(const String& dumb);
