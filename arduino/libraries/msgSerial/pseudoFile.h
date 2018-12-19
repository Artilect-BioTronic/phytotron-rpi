/*---------------------------------------------------------------*/
/*   This code was used to test the commands for a SD card       */
/*     without connecting a SDcard module.                       */
/*   But it was not very effective and it is not used often      */
/*---------------------------------------------------------------*/

#include "Arduino.h"

//#include "SD.h"
//#include "SdFat.h"
//extern SdFat SD;

#include "msgSerial.h"

class PseudoFile: public Stream
{
public:
    int open(const String& fileName);
    int close();
    PseudoFile(): lineNum_(0), pos_(0), isOpened_(false) {}

    int available() ;
    int read() ;
    int peek() ;
    void flush() {}
    // abstract function in Stream, inherited from Print
    size_t write(uint8_t)   {return 0;}


private:
    const int size_ = 100;
    int lineNum_;
    int pos_;   // position in file
    boolean isOpened_;

    int generateCharInFile(int aPos);
};
