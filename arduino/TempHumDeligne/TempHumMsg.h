#ifndef TEMPHUMMSG_H
#define TEMPHUMMSG_H

#include "Arduino.h"

#include "msgSerial.h"

#define CFG_CPLT        1
#define CFG_SD_RTC      2

#define CFG_MAT_CHECK   CFG_SD_RTC

#ifdef CFG_MAT
    #if (CFG_MAT != CFG_MAT_CHECK)
        #error "CFG_MAT_CHECK" and "CFG_MAT" must be updated with same value
    #endif
#else
    #define CFG_MAT     CFG_MAT_CHECK
#endif

#if (CFG_MAT == CFG_CPLT)
#define SERIAL_MSG Serial1
#else
#define SERIAL_MSG Serial
#endif

#include "msgExampleFunction.h"

/*---------------------------------------------------------------*/
/*                                          */
/*---------------------------------------------------------------*/

// liste des fonctions definies dans le fichier TempHumDeligne.ino
String numeroDix ( int valeur );
void erreur ( byte numeroErreur );
void releveRTC ( void );
void afficheLCD ( String texte , unsigned int positionColonne , boolean ligne  );
void releveValeurs ( void );
void afficheLCDregulier ( void );
void mesureTemperature18B20 (void); //Dallas
void dump ( String nomFichier );
void dumpRasp ( String nomFichier );
void commandeSwitch ( int valeur );
void affichageUsbSecondes ( void );
void affichageSerieRaspSecondes ( void );
void FonctionTexteTrameMesures ( void );
String getTexteEnteteMesures ( void );
void EnregistrementFichierMesure ( void );
int lectureSerialUSB_PM();
int lectureSerialRaspi_PM();
int ecritConsigneDansFichier();
int sendConsigne();
String getTrameConsigne();
int sendCmdState(String warm, String cold, String humidity);

// liste des fonctions definies dans le fichier TempHumMsg.cpp
int sendDate(const CommandList& aCL, Command &aCmd, const String& aInput);
int updateHumCsgn(const CommandList& aCL, Command &aCmd, const String& aInput);
int updateTempCsgn(const CommandList& aCL, Command &aCmd, const String& aInput);

void     fakeReleveValeurs();


// liste des variables globales definies dans le fichier TempHumDeligne.ino
extern int consigneTemp;
extern byte consigneHum ;

extern byte temperatureInterieureEntiere ;
extern byte humiditeInterieureEntiere ;
extern byte temperatureExterieureEntiere ;
extern byte humiditeExterieureEntiere ;


// liste des variables dans le fichier TempHumMsg.cpp
extern SerialListener serListenerTH;

// list of available commands (user) that the arduino will accept
extern CommandList cmdLUserPhy;  // ("cmdUser", "CM+", SIZE_OF_TAB(cmdUserPhy), cmdUserPhy );
// list of available commands (system ctrl) that the arduino will accept
extern CommandList cmdLSysPhy;   //("cmdSys", "AT+", SIZE_OF_TAB(cmdSysPhy), cmdSysPhy );

int setupTempHumMsg();

int changeNameVal(const String& astr);

class Chauffage
{
private:
    uint8_t _pinRad1;
    uint8_t _pinRad2;
    uint8_t _pinFan;
    boolean _isOn;
    uint8_t _pctChauffe;

public:
    Chauffage(uint8_t pinRad1, uint8_t pinRad2, uint8_t pinFan, int pctChauffe);
    int switchOn(void);
    int switchOff(void);
    boolean isOn(void)   {return _isOn;}
    boolean isOff(void)   {return ! _isOn;}
};

#endif // TEMPHUMMSG_H
