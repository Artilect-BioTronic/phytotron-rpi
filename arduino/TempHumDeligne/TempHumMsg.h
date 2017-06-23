#include "Arduino.h"

//#include "SD.h"
//#include "SdFat.h"
//extern SdFat SD;

#include "msgSerial.h"

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
void EnregistrementFichierMesure ( void );
int lectureSerialUSB_PM();
int lectureSerialRaspi_PM();
int ecritConsigneDansFichier();
int sendConsigne();
String getTrameConsigne();

// liste des fonctions definies dans le fichier TempHumMsg.cpp
int sendDate(const String& aStr);
int updateHumCsgn(const String& aStr);
int updateTempCsgn(const String& aStr);

void     fakeReleveValeurs();


// liste des variables globales definies dans le fichier TempHumDeligne.ino
extern byte consigneTemp;
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
