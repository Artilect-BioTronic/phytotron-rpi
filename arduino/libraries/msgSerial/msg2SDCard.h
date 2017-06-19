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

    int readlnNoOpen(String& aString);
    int readNcharNoOpen(String& strBuf, int nbChar);
    long moveToNoOpen(const String& aString);

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
    int readln(String& aString);
    int writeln(const String& aString);
    long moveTo(const String& aString);
    int rename(const String& aOld, const String& aNew);
    int remove(const String& aString);
    int mkdir(const String& aString);
    int ls(Stream &aStream, const uint8_t aOption);
    Cmd2File();
    ~Cmd2File();
};


/** Prepare to open a file before read / write can be done
 *
 * \param[in] argFile Path name to the file
 *
 * The file stays open until call to srClose
 * You d better use  srPreOpen
 *
 * \return 0 for success and negative for failure
 */
int srStayOpen(const String& argFile);

/** Prepare to open a file before read / write can be done
 *
 * \param[in] argFile Path name to the file
 *
 * There is an attempt to open the file, then it is closed.
 * At each call to srReadNchar / srWriteln the file will be open
 * temporarily and it will be closed immediately
 *
 * \return 0 for success and negative for failure
 */
int srPreOpen(const String& argFile);

int srClose(const String& argFile);

int srReadln(const String& dumb);

int srWriteln(const String& aToWrite);

int srMove(const String& a2Search);

int srReadNchar(const String& aString);

int srRename(const String& aString);

int srRemove(const String& aFilename);

int srLs(const String& arg);

int srMkdir(const String& aString);

int srDump2(const String& aString);
