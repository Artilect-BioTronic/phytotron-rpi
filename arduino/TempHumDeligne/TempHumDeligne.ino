// Phytotron
// régulation température/humidité...
// pour Mega et carte Memoire de Snootlab
// Ne pas utiliser la pin 13 pour la led
// Relier pins pour compatibilite Mega/Memoire
//SDA   18-20
//SCL   19-21
//SCK   13-52
//MISO  12-50
//MOSI  11_51
//SS    10-53


// Bibliothéques
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Deuligne.h>
#include "Adafruit_HTU21DF.h"
#include <DHT.h>
#include <DHT_U.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <SPI.h>
//#include <SD.h>
#include "SdFat.h"
extern SdFat SD;
#include <RCSwitch.h>
#include <OneWire.h>

#define CFG_CPLT        1
#define CFG_SD_RTC      2


#define CFG_MAT  CFG_CPLT

#if (CFG_MAT == CFG_CPLT)
#define SERIAL_MSG Serial1
#else
#define SERIAL_MSG Serial
#endif

#include "msgSerial.h"
#include "TempHumMsg.h"


const uint8_t pinRad1 = 30;
const uint8_t pinRad2 = 31;
const uint8_t pinFan = 32;
const uint8_t pinStick = A0 ; // stick du deuligne pour menu
//const int chipSelect = 10 ; // uno
const uint8_t chipSelect = 53 ; // mega   (SPI pour SDCard)
const uint8_t CapteurHumiditeTemperatureExterieurPIN = 2; // pin capteur humidite
const uint8_t CapteurHumiditeTemperatureInterieurPIN = 40; // pin capteur humidite
const uint8_t PinOneWire = 69 ; // pin capteur temperature Dallas
const uint8_t pinTelecommande = 7 ;

// Si vous n avez pas de shield deuligne, il faut connecter pinStick a 5V,
//   sinon le sketch voit des actions permanentes pour naviguer dans le menu
// Si vous n utilisez pas le serial1, il faut bouchonner rx

// example of frame:
// -t "/phytotron/cli/mesure/D;H;ti;hi;te;he;t3;t2;t1"
// -m "2017-05-08;12:34:56;11;12.1;21.1;22.1;33.1;32.1;31.1"
const String topicMesure        = "mesure/D;H;ti;hi;te;he;t3;t2;t1";
const String topicConsigneState = "csgn/state/D;H;t;h";
const String topicCmdState = "cmd/state/D;H;warm;cold;hum";
const String head1="CM+";
const String endOL="\n";

String fmt1CmdMqtt(const String & aTopic, const String& aLoad)
{
    return head1 + aTopic + ":" + aLoad + endOL;
}

#if (CFG_MAT == CFG_CPLT)
const long serial1Rate=   115200;
const long serialRate=    38400;
#else
const long serial1Rate=   115200;
const long serialRate=    38400;
#endif

//reglages
const byte intervalleEnregistrement = 60 ; // en secondes
const byte intervalleAffichage = 1 ; // en secondes
const byte nbCommandeSwitch = 5 ;
const int humidificateurMarche = 5393 ;
const int humidificateurArret = 5396 ;
const int refroidissementMarche = 4433 ;
const int refroidissementArret = 4436 ;
//const int chauffageMarche = 1361 ;
//const int chauffageArret = 1364 ;


// DeuLigne
byte degres[8] =
{
  B00100,
  B01010,
  B01010,
  B00100,
  B00000,
  B00000,
  B00000
} ;
Deuligne lcd ;
const String effacement = "                " ;
// variables et constantes pour menu
const unsigned long delaisMenu = 5000 ;
unsigned int valeurStick = 0 ;
boolean menu = false ;
boolean selection = false ;
boolean validation = false ;
unsigned long debutMenu = 0 ;
boolean reglageTemp = false ;
boolean reglageHum = false ;
int consigneTemp = 20 ;
int consigneTempProvisoire = 20 ;
byte plageTemp = 1 ;
byte consigneHum = 50 ;
byte consigneHumProvisoire = 50 ;
byte plageHum = 5 ;
//bool commandeChauffage = false ;  // use chauffage.isOn()
bool commandeRefroidissement = false ;
bool commandeHum = false ;
unsigned long positionFichierConsignes ;
Chauffage chauffage(pinRad1, pinRad2, pinFan, 50);
ChaudCoexisteAvecFroid frigoContreChauffage(plageTemp);

//Génération des trames
String TrameMesures = "" ;
const String separateurFichier = ";" ;

//enregistrement SD sur Memoire
//noms des fichiers
const String  NomFichierConsignes  = "cons.csv" ;
const String  NomFichierMesure  = "mes.csv" ;
const String  NomFichierCommandes  = "com.csv" ;
const unsigned long  positionEnregistrementFichierConsignes  = 7 ; // position avant derniere consigne temperature

//Echanges de fichiers
const String debutFichier = "Debut du fichier" ;
const String finFichier = "Fin du fichier" ;
const String  commandeLectureFichierConsignes  = "cons" ;
const String  commandeLectureFichierMesure  = "mes" ;
const String  commandeLectureFichierCommandes  = "com" ;



//Capteurs temperature-humidite
//Exterieur
// Uncomment the type of sensor in use:
#define CapteurHumiditeTemperatureExterieurTYPE           DHT11     // DHT 11
//#define CapteurHumiditeTemperatureExterieurTYPE           DHT22     // DHT 22 (AM2302)
//#define CapteurHumiditeTemperatureExterieurTYPE           DHT21     // DHT 21 (AM2301)
DHT_Unified CapteurHumiditeTemperatureExterieur( CapteurHumiditeTemperatureExterieurPIN , CapteurHumiditeTemperatureExterieurTYPE ) ;


//Interieur
// Uncomment the type of sensor in use:
//#define CapteurHumiditeTemperatureInterieurTYPE           DHT11     // DHT 11
#define CapteurHumiditeTemperatureInterieurTYPE           DHT22     // DHT 22 (AM2302)
//#define CapteurHumiditeTemperatureInterieurTYPE           DHT21     // DHT 21 (AM2301)
DHT_Unified CapteurHumiditeTemperatureInterieur( CapteurHumiditeTemperatureInterieurPIN , CapteurHumiditeTemperatureInterieurTYPE ) ;


//Adafruit_HTU21DF htu = Adafruit_HTU21DF();
//boolean HTU21 = false ; // présence capteur HTU21

//capteur(s) temperature Dallas
OneWire  Ds18b20 ( PinOneWire ) ;
const byte nbCapteurs = 3 ;
FilterDallas dallasFiltered[ nbCapteurs ] = {15., 15., 15.} ;
//byte present = 0;
//byte type_s;
//byte data[12];
//byte addr[8];
//byte noCapteur = 0 ; // pour éventuellement compter le nombre de capteurs
//byte i=0;
//byte phase = 0 ;
//int mesure = 0 ;
unsigned long Temps = 0 ;
unsigned long TempsMesure = 0 ;
unsigned long TempsEnregistrement = 0 ;
unsigned long TempsMesurePrecedent = 0 ;
unsigned long TempsEnregistrementPrecedent = 0 ;



//Prises telecommandees
RCSwitch telecommande = RCSwitch ( ) ;

// Variables pour temps et mesures
byte temperatureInterieureEntiere ;
byte humiditeInterieureEntiere ;
byte temperatureExterieureEntiere ;
byte humiditeExterieureEntiere ;
String heuresDix ;
String minutesDix ;
String secondesDix ;
String heureString ;
String dateString ;
unsigned long tempsEnregistrement = 0 ;
byte minutes = 0 ;
byte secondesAffichagePrecedent = 0 ;
byte secondes = 0 ;

// Variables pour communication serie
//char lecturePortSerie ;
//String messageRecu = "" ;
//String messageRecuUSB = "" ;
//String messageRecuRaspi = "" ;

void setup(void)
{
  Wire.begin ( ) ;
  telecommande.enableTransmit ( pinTelecommande ) ;
  commandeSwitch ( humidificateurArret ) ;
  commandeSwitch ( refroidissementArret ) ;
  chauffage.switchOff();
  Serial1.begin ( serial1Rate ) ;
  Serial.begin ( serialRate ) ;
  while ( !Serial )
  {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println ( "Debut de l'init" ) ;

  // I fill info on sketch
  sketchInfo.setFileDateTime(F(__FILE__), F(__DATE__), F(__TIME__));
  // on  setup  la librairie de communication
  setupTempHumMsg();

  lcd.init ( ) ;
  lcd.createChar ( 0 , degres ) ;
  CapteurHumiditeTemperatureInterieur.begin ( ) ;
  CapteurHumiditeTemperatureExterieur.begin ( ) ;

  releveRTC ( ) ; //on releve date et heure sur l'horloge RTC
  sensor_t sensor; //Initialise the sensor
  CapteurHumiditeTemperatureInterieur.temperature().getSensor(&sensor);
  CapteurHumiditeTemperatureInterieur.humidity().getSensor(&sensor);
  CapteurHumiditeTemperatureExterieur.temperature().getSensor(&sensor);
  CapteurHumiditeTemperatureExterieur.humidity().getSensor(&sensor);

  /*  if (!htu.begin())
    {
      Serial.println("Capteur HTU21 absent");
      HTU21 = false ;
      //while (1);
    }
    else
    {
      HTU21 = true ;
    }

  */



  //initialisation carte SD
  if ( !SD.begin ( chipSelect ) )
  {
    erreur ( 3 ) ; //carte SD non detecte
  }
  else
  {
    File dataFile = SD.open ( NomFichierMesure , FILE_WRITE ) ;
    if (dataFile)
    {
      dataFile.println ( getTexteEnteteMesures() ) ;
      dataFile.close();
    }
    else
    {
      erreur ( 4 ) ; //non ecriture fichier mesure sur carte SD
    }

    File regFile = SD.open ( NomFichierConsignes , FILE_READ ) ;
    if (regFile)
    {
      positionFichierConsignes = regFile.size ( ) ;
      positionFichierConsignes = positionFichierConsignes - positionEnregistrementFichierConsignes ;
      regFile.seek ( positionFichierConsignes ) ;
      consigneTemp = ( ( regFile.read ( ) - 48 ) * 10 ) + ( regFile.read ( ) - 48 ) ;
      regFile.read ( ) ;
      consigneHum = ( ( regFile.read ( ) - 48 ) * 10 ) + ( regFile.read ( ) - 48 ) ;
      regFile.close ( ) ;
    }
    else
    {
      erreur ( 5 ) ;  //non lecture fichier consigne sur carte SD
    }

    File regFileEcriture = SD.open ( NomFichierConsignes , FILE_WRITE ) ;
    if (regFileEcriture)
    {
      regFileEcriture.print ( "Date" ) ;
      regFileEcriture.print ( separateurFichier ) ;
      regFileEcriture.print ( "Heure" ) ;
      regFileEcriture.print ( separateurFichier ) ;
      regFileEcriture.print ( "Consigne_Temperature" ) ;
      regFileEcriture.print ( separateurFichier ) ;
      regFileEcriture.print ( "Consigne_Humidite" ) ;
      regFileEcriture.println ( "" ) ;
      regFileEcriture.close();
    }
    else
    {
      //non ecriture fichier consigne sur carte SD
      erreur ( 6 ) ;
    }
  }

  File regFileEcriture = SD.open ( NomFichierConsignes , FILE_WRITE ) ;
  if ( regFileEcriture )
  {
    regFileEcriture.print ( dateString ) ;
    regFileEcriture.print ( separateurFichier ) ;
    regFileEcriture.print ( heureString ) ;
    regFileEcriture.print ( separateurFichier ) ;
    regFileEcriture.print ( numeroDix ( consigneTemp ) ) ;
    regFileEcriture.print ( separateurFichier ) ;
    regFileEcriture.print ( numeroDix ( consigneHum ) ) ;
    regFileEcriture.println ( "" ) ;
    regFileEcriture.close ( ) ;
  }
  else
  {
    //non ecriture fichier consigne sur carte SD
    erreur ( 7 ) ;
  }

  File dataFile = SD.open ( NomFichierCommandes , FILE_WRITE ) ;
  if (dataFile)
  {
    dataFile.print ( "Date" ) ;
    dataFile.print ( separateurFichier ) ;
    dataFile.print ( "Heure" ) ;
    dataFile.print ( separateurFichier ) ;
    dataFile.print ( "Chauffage" ) ;
    dataFile.print ( separateurFichier ) ;
    dataFile.print ( "Refroidissement" ) ;
    dataFile.print ( separateurFichier ) ;
    dataFile.print ( "Humidification" ) ;
    dataFile.println ( "" ) ;
    dataFile.close();
  }
  else
  {
    erreur ( 8 ) ; //non ecriture fichier mesure sur carte SD
  }

  releveRTC ( ) ; //on releve date et heure sur l'horloge RTC
  releveValeurs ( ) ;
  afficheLCDregulier ( ) ;
  secondesAffichagePrecedent = secondes ;
  // tempsEnregistrement correspondra au moment du dernier enregistrement, 1 par mn
  // je prefere arrondir tempsEnregistrement a une mn entiere
  tempsEnregistrement = millis ( ) - secondes*1000 ;
}


/*=====================================*/
/*           fonctions  loop           */
/*=====================================*/

void loop ( )
{
  Temps = millis ( ) ;
  releveRTC ( ) ; //on releve date et heure sur l'horloge RTC
  mesureTemperature18B20 ( ) ;

  // enregistrement regulier
  //   (on en profite pour repeter les consignes)
  if ( (millis() - tempsEnregistrement)/1000 > intervalleEnregistrement)
  {
      tempsEnregistrement = millis();
      EnregistrementFichierMesure ( ) ;

      // we make sure that consigns are updated
      sendConsigne();

      // we regularly re apply consignes to switches
      if (commandeRefroidissement)
          commandeSwitch ( refroidissementMarche ) ;
      else
          commandeSwitch ( refroidissementArret ) ;

      if (chauffage.isOn())
          chauffage.switchOn();
      else
          chauffage.switchOff();

      if (commandeHum)
          commandeSwitch ( humidificateurMarche) ;
      else
          commandeSwitch ( humidificateurArret) ;
  }

  // affichage regulier
  if ( ( secondes !=  secondesAffichagePrecedent ) && ! menu )
  {
    secondesAffichagePrecedent = secondes ;
    releveValeurs ( ) ;
//    EnregistrementFichierMesure ( ) ;
    FonctionTexteTrameMesures ( ) ;
    afficheLCDregulier ( ) ;
    affichageUsbSecondes ( ) ;
    affichageSerieRaspSecondes ( ) ;
  }


  //delais menu
  if ( ( millis ( ) - debutMenu > delaisMenu ) && menu )
  {
    menu = false ;
    selection = false ;
    //Serial.println ( "fin menu" ) ;
    lcd.noBlink ( ) ;
    afficheLCD ( effacement , 0, 0 ) ;
    afficheLCD ( effacement , 0, 1 ) ;
  }

  // lecture commande stick
  valeurStick = analogRead ( pinStick ) ;
  if ( valeurStick < 1013 )
  {
    debutMenu = millis ( ) ;
    if ( ! menu )
    {
      //Serial.println ( "entree menu" ) ;
      afficheLCD ( effacement , 0 , 0 ) ;
      afficheLCD ( "Reglages" , 4 , 0 ) ;
      afficheLCD ( effacement , 0 , 1 ) ;
      afficheLCD ( String ( consigneTemp ) , 0 , 1 ) ;
      lcd.write ( 0 ) ;
      lcd.print ( "C" ) ;
      afficheLCD ( "C" , 3 , 1 ) ;
      afficheLCD ( String ( consigneHum ) , 13 , 1 ) ;
      afficheLCD ( "%" , 15, 1 ) ;
      afficheLCD ( "" , 7, 1 ) ;
      lcd.blink ( ) ;
      menu = true ;
      consigneTempProvisoire = consigneTemp ;
      consigneHumProvisoire = consigneHum ;
    }

    if ( valeurStick < 10 && selection ) // droite
    {
      lcd.setCursor ( 15 , 1 ) ;
      reglageHum = true ;
      reglageTemp = false ;
    }

    if ( 665 < valeurStick && valeurStick < 685 && selection  ) // gauche
    {
      lcd.setCursor ( 2 , 1 ) ;
      reglageHum = false ;
      reglageTemp = true ;
    }

    if ( 241 < valeurStick && valeurStick < 261 && selection  ) // haut
    {
      if ( reglageTemp )
      {
        consigneTempProvisoire ++ ;
        lcd.setCursor ( 0 , 1 ) ;
        lcd.print ( numeroDix ( consigneTempProvisoire ) ) ;
      }
      if ( reglageHum )
      {
        consigneHumProvisoire ++ ;
        lcd.setCursor ( 13 , 1 ) ;
        lcd.print ( numeroDix ( consigneHumProvisoire ) ) ;
      }
      selection = false ;
    }

    if ( 487 < valeurStick && valeurStick < 504 && selection  ) // bas
    {
      if ( reglageTemp )
      {
        consigneTempProvisoire -- ;
        lcd.setCursor ( 0 , 1 ) ;
        lcd.print ( numeroDix ( consigneTempProvisoire ) ) ;
      }

      if ( reglageHum )
      {
        consigneHumProvisoire -- ;
        lcd.setCursor ( 13 , 1 ) ;
        lcd.print ( numeroDix ( consigneHumProvisoire ) ) ;
      }
      selection = false ;
    }

    if ( 850 < valeurStick && valeurStick < 870 && selection && ! validation ) // appuye
    {
      //enregistre les consignes
      consigneHum = consigneHumProvisoire ;
      consigneTemp = consigneTempProvisoire ;
      validation = true ;
      //Serial.println ( "appuye" ) ;
    }
  }

  if ( menu && ! selection && (valeurStick > 1013) )
  {
    selection = true ;
    //Serial.println ( "selection" ) ;
  }

  if ( menu && selection && validation && (valeurStick > 1013) )
  {
    menu = false ;
    selection = false ;
    validation = false ;
    //Serial.println ( "enregistre" ) ;
    lcd.noBlink ( ) ;
    lcd.setCursor ( 0 , 0 ) ;
    lcd.print ( effacement ) ;
    lcd.setCursor ( 0 , 1 ) ;
    lcd.print ( effacement ) ;
    lcd.setCursor ( 1 , 0 ) ;
    lcd.print ( "Enregistrement" ) ;
    delay ( 2000 ) ;

    ecritConsigneDansFichier();
    sendConsigne();

    lcd.setCursor ( 0 , 0 ) ;
    lcd.print ( effacement ) ;
    lcd.setCursor ( 3 , 0 ) ;
    lcd.print ( "Enreg. OK" ) ;
    delay ( 2000 ) ;
    lcd.setCursor ( 0 , 0 ) ;
    lcd.print ( effacement ) ;
  }


  //*********************************
  //       Asservissements
  //*********************************

  // Humidite prise " D"
  if ( (humiditeInterieureEntiere < ( consigneHum - plageHum )) & ! commandeHum )
  {
    //telecommande.send ( 5393 , 24 ) ; // marche
    commandeSwitch ( humidificateurMarche ) ;
    commandeHum = true ;

    // previent du changement de commande
    sendCmdState("","", "Marche_Humidification");
    File comFile = SD.open ( NomFichierCommandes , FILE_WRITE ) ;
    if ( comFile )
    {
      comFile.print ( dateString ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( heureString ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( "" ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( "" ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( "Marche_Humidification" ) ;
      comFile.println ( "" ) ;
      comFile.close();
    }
    else
    {
      //erreur sur ecriture recurente carte SD
      erreur ( 12 ) ;
    }
  }
  if ( (humiditeInterieureEntiere > consigneHum) & commandeHum )
  {
    //telecommande.send ( 5396 , 24 ) ; // arret
    commandeSwitch ( humidificateurArret ) ;
    commandeHum = false ;

    // previent du changement de commande
    sendCmdState("","", "Arret_Humidification");
    File comFile = SD.open ( NomFichierCommandes , FILE_WRITE ) ;
    if ( comFile )
    {
      comFile.print ( dateString ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( heureString ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( "" ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( "" ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( "Arret_Humidification" ) ;
      comFile.println ( "" ) ;
      comFile.close();
    }
    else
    {
      //erreur sur ecriture recurente carte SD
      erreur ( 13 ) ;
    }
  }

  // Attention, On ne veut pas que le refroidissement du frigo ne declenche le chauffage
  //   ici on penalise le chauffage car on est en ete
  // Chauffage prise " A "
  if ( temperatureInterieureEntiere < ( consigneTemp - plageTemp ) &&
       chauffage.isOff()   &&
       frigoContreChauffage.isOldFinFroid(consigneTemp, temperatureInterieureEntiere) )
  {
    chauffage.switchOn();
    frigoContreChauffage.timeChaudActif();

    // previent du changement de commande
    sendCmdState("Marche_Chauffage","", "");
    File comFile = SD.open ( NomFichierCommandes , FILE_WRITE ) ;
    if ( comFile )
    {
      comFile.print ( dateString ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( heureString ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( "Marche_Chauffage" ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( "" ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( "" ) ;
      comFile.println ( "" ) ;
      comFile.close();
    }
    else
    {
      //erreur sur ecriture recurente carte SD
      erreur ( 14 ) ;
    }
  }

  // arret chauffage
  if ( temperatureInterieureEntiere > ( consigneTemp + plageTemp * 0. ) && chauffage.isOn() )
  {
    chauffage.switchOff();
    frigoContreChauffage.timeChaudActif();

    // previent du changement de commande
    sendCmdState("Arret_Chauffage","", "");
    File comFile = SD.open ( NomFichierCommandes , FILE_WRITE ) ;
    if ( comFile )
    {
      comFile.print ( dateString ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( heureString ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( "Arret_Chauffage" ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( "" ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( "" ) ;
      comFile.println ( "" ) ;
      comFile.close();
    }
    else
    {
      //erreur sur ecriture recurente carte SD
      erreur ( 15 ) ;
    }
  }

  // declenchement Refroidissement, prise " B "
  if ( temperatureInterieureEntiere > ( consigneTemp + plageTemp ) &&
       ! commandeRefroidissement  &&
       frigoContreChauffage.isOldFinChaud(consigneTemp, temperatureInterieureEntiere) )
  {
    commandeSwitch ( refroidissementMarche ) ;
    commandeRefroidissement = true ;
    frigoContreChauffage.timeFroidActif();

    // previent du changement de commande
    sendCmdState("","Marche_Refroidissement", "");
    File comFile = SD.open ( NomFichierCommandes , FILE_WRITE ) ;
    if ( comFile )
    {
      comFile.print ( dateString ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( heureString ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( "" ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( "Marche_Refroidissement" ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( "" ) ;
      comFile.println ( "" ) ;
      comFile.close();
    }
    else
    {
      //erreur sur ecriture recurente carte SD
      erreur ( 16 ) ;
    }
  }
  // arret refroidissement
  if ( temperatureInterieureEntiere < ( consigneTemp - plageTemp*0. ) && commandeRefroidissement )
  {
    commandeSwitch ( refroidissementArret ) ;
    commandeRefroidissement = false ;
    frigoContreChauffage.timeFroidActif();

    // previent du changement de commande
    sendCmdState("","Arret_Refroidissement", "");
    File comFile = SD.open ( NomFichierCommandes , FILE_WRITE ) ;
    if ( comFile )
    {
      comFile.print ( dateString ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( heureString ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( "" ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( "Arret_Refroidissement" ) ;
      comFile.print ( separateurFichier ) ;
      comFile.print ( "" ) ;
      comFile.println ( "" ) ;
      comFile.close();
    }
    else
    {
      //erreur sur ecriture recurente carte SD
      erreur ( 17 ) ;
    }
  }

  serListenerTH.checkMessageReceived();

#if (CFG_MAT == CFG_CPLT)
  lectureSerialUSB_PM();
#endif

  // Lecture port serie RasbPi  -->  remplace par serListenerTH
//  lectureSerialRaspi_PM() ;

}

/*=====================================*/
/*           liste de fonctions        */
/*=====================================*/

// Lecture port serie USB
int lectureSerialUSB_PM()
{
    // pas de communication en cours, on s en va de suite
    if ( Serial.available ( ) <= 0 )
        return 0;

    delay(10); // give some time to receive msg
    String messageRecuUSB = "" ;
    while ( Serial.available ( ) > 0)
    {
        char lecturePortSerieUSB = Serial.read ( ) ;
        messageRecuUSB = String ( messageRecuUSB + lecturePortSerieUSB ) ;
    }
    Serial.print ( "messageRecuUSB =>" ) ;   // send an initial string
    Serial.print ( messageRecuUSB ) ;   // send an initial string
    Serial.println ( "<=" ) ;   // send an initial string
    if ( messageRecuUSB == commandeLectureFichierConsignes )
    {
        dump ( NomFichierConsignes ) ;
    }
    if ( messageRecuUSB == commandeLectureFichierMesure )
    {
        dump ( NomFichierMesure ) ;
    }
    if ( messageRecuUSB == commandeLectureFichierCommandes )
    {
        dump ( NomFichierCommandes ) ;
    }
    if ( messageRecuUSB == "st" )
    {
        affichageUsbSecondes() ;
    }
    messageRecuUSB = "" ;

    return 0;
}

// Lecture port serie RasbPi
int lectureSerialRaspi_PM()
{
    // pas de communication en cours, on s en va de suite
    if ( Serial1.available ( ) <= 0 )
        return 0;

    delay(10); // give some time to receive msg
    String messageRecuRaspi = "" ;
    while ( Serial1.available ( ) > 0)
    {
        char lecturePortSerieRaspi = Serial1.read ( ) ;
        messageRecuRaspi = String ( messageRecuRaspi + lecturePortSerieRaspi ) ;
    }
    Serial.print ( "messageRecu =>" ) ;   // send an initial string
    Serial.print ( messageRecuRaspi ) ;   // send an initial string
    Serial.println ( "<=" ) ;   // send an initial string
    if ( messageRecuRaspi == commandeLectureFichierConsignes )
    {
        dumpRasp ( NomFichierConsignes ) ;
    }
    if ( messageRecuRaspi == commandeLectureFichierMesure )
    {
        dumpRasp ( NomFichierMesure ) ;
    }
    if ( messageRecuRaspi == commandeLectureFichierCommandes )
    {
        dumpRasp ( NomFichierCommandes ) ;
    }
    if ( messageRecuRaspi == "st" )
    {
        affichageSerieRaspSecondes() ;
    }
    messageRecuRaspi = "" ;

    return 0;
}

//fonction permettant de transformer un integer en chaine de 2 caracteres ( zero en premier par defaut )
String numeroDix ( int valeur )
{
  if ( valeur >= 0 && valeur < 10)
  {
    return String ( "0" + String ( valeur ) ) ;
  }
  else
  {
    return String ( valeur ) ;
  }
}

//fonction d'affichage d'erreurs sur liaison serie
void erreur ( byte numeroErreur )
{
  Serial.print ( "Erreur N " ) ;
  Serial.print ( numeroErreur ) ;
  Serial.println ( "" ) ;
}

//fonction de releve heure RTC
void releveRTC ( void )
{
  tmElements_t tm;
  if ( RTC.read ( tm ) )
  {
    minutes = tm.Minute ;
    secondes = tm.Second ;
    heureString = String ( numeroDix ( tm.Hour ) )
                           + ":"
                           + numeroDix ( minutes )
                           + ":"
                           + numeroDix ( secondes ) ;

    dateString = String ( tmYearToCalendar ( tm.Year ))
                          + "-"
                          + numeroDix ( tm.Month )
                          + "-"
                          + numeroDix ( tm.Day ) ;
  }
}

//fonction d'affichage LCD
void afficheLCD ( String texte , unsigned int positionColonne , boolean ligne  )
{
  lcd.setCursor ( positionColonne , ligne ) ;
  lcd.print ( texte ) ;
}

int sendConsigne()   {
    String sTrame = getTrameConsigne();
    Serial.print( fmt1CmdMqtt (topicConsigneState, sTrame) );
    Serial1.print( fmt1CmdMqtt (topicConsigneState, sTrame) );
    return 0;
}

int sendCmdState(String warm, String cold, String humidity)   {
    // topicCmdState = "cmd/state/D;H;warm;cold;hum";
    String sTrame = dateString   +separateurFichier+
            heureString   +separateurFichier+
            warm  +separateurFichier+
            cold  +separateurFichier+
            humidity  ;
    Serial.print( fmt1CmdMqtt (topicCmdState, sTrame) );
    Serial1.print( fmt1CmdMqtt (topicCmdState, sTrame) );
    return 0;
}

String getTrameConsigne() {
    String sTrame = "";
    // String(floatValue)  will convert float to String with format %0.2f
    sTrame = dateString   +separateurFichier+
             heureString  +separateurFichier+
             String ( consigneTemp )  +separateurFichier+
             String ( consigneHum );
    return sTrame;
}

int ecritConsigneDansFichier()   {
    File regFile = SD.open ( NomFichierConsignes , FILE_WRITE ) ;
    if ( regFile )
    {
        regFile.println ( getTrameConsigne() ) ;
        regFile.close();
        return 0;
    }
    else
    {
        //non ecriture fichier consigne sur carte SD
        erreur ( 10 ) ;
        return 10;
    }
}

void releveValeurs ( void )
{
#if (CFG_MAT != CFG_CPLT)
    // Je suppose qu on n a pas des vrais capteurs connectes
    // j en produits des fausses et je m en vais
    fakeReleveValeurs();
    return;
#endif
  sensors_event_t event; // Lancer les mesures
  //Interieur
  CapteurHumiditeTemperatureInterieur.temperature().getEvent(&event); //temperature par DHT11
  if (isnan(event.temperature))
  {
    erreur ( 10 ) ;
  }
  else
  {
    temperatureInterieureEntiere = event.temperature ;
  }
  CapteurHumiditeTemperatureInterieur.humidity ( ) . getEvent ( &event ) ; //humidite par DHT11
  if ( isnan( event . relative_humidity ) )
  {
    erreur ( 11 ) ; ;
  }
  else
  {
    humiditeInterieureEntiere = event.relative_humidity ;
  }


  //exterieur
  CapteurHumiditeTemperatureExterieur.temperature().getEvent(&event); //temperature par DHT11
  if (isnan(event.temperature))
  {
    erreur ( 10 ) ;
  }
  else
  {
    temperatureExterieureEntiere = event.temperature ;
  }
  CapteurHumiditeTemperatureExterieur.humidity ( ) . getEvent ( &event ) ; //humidite par DHT11
  if ( isnan( event . relative_humidity ) )
  {
    erreur ( 11 ) ; ;
  }
  else
  {
    humiditeExterieureEntiere = event.relative_humidity ;
  }
  /*  if ( HTU21 )
    {
    //    temperatureInterieureEntiere = int ( htu.readTemperature ( ) ) ;
    float




      //humiditeInterieureEntiere = int ( htu.readHumidity ( ) ) ;
    }
    else
    {
      temperatureInterieureEntiere = 00 ;
      humiditeInterieureEntiere = 00 ;
    }
  */
}

void afficheLCDregulier ( void )
{
  afficheLCD ( numeroDix ( temperatureInterieureEntiere ) , 0 , 0 ) ;
  lcd.setCursor ( 2 , 0 ) ;
  lcd.write ( 0 ) ;
  afficheLCD ( "C" , 3 , 0 ) ;
  afficheLCD ( dateString , 6 , 0 ) ;
  afficheLCD ( numeroDix ( humiditeInterieureEntiere ) , 0 , 1 ) ;
  afficheLCD ( "%" , 2 , 1 ) ;
  afficheLCD ( heureString , 8 , 1 ) ;
}





void mesureTemperature18B20 (void) //Dallas
{
    static byte present = 0;
    static byte type_s;
    byte data[12];
    static byte addr[8];
    static byte noCapteur = 0 ; // pour éventuellement compter le nombre de capteurs
    byte i=0;
    static byte phase = 0 ;
    int mesure = 0 ;

  if ( phase == 0 && ( Temps - TempsMesurePrecedent ) > 150  ) // a chaque nouveau capteur et apres un certain temps
  {
    TempsMesurePrecedent = Temps; // on lance le chrono
    if ( !Ds18b20.search(addr)) // est-ce que le dernier est passé ?
    {
      noCapteur = 0 ; // on recommence au debut
      Ds18b20.reset_search ( ) ; // on demande l'identification
      //delay ( 10 ) ;
//      Serial.println(String("DSresetS"));
    }
    else // il y a un capteur a mesurer
    {
      phase = 1 ; // au coup suivant on ne passeras pas par la
      noCapteur ++ ; // un de plus
//      Serial.println(String("DSnewC:")+ noCapteur);
    }
  }
  if ( phase == 1 ) // bon, on va mesurer enfin
  {
    switch (addr[0]) // premiere adresse de capteur
    {
      case 0x10:
        //       Serial.println("  Chip = DS18S20");  // or old DS1820
        type_s = 1;
        break;
      case 0x28:
        //       Serial.println("  Chip = DS18B20");
        type_s = 0;
        break;
      case 0x22:
        //       Serial.println("  Chip = DS1822");
        type_s = 0;
        break;
      default:
        //       Serial.println("Device is not a DS18x20 family device.");
        //       temperature18B20 = String ( "Erreur Identification capteur" ) ;
        return;
    }
    Ds18b20.reset(); // on change de capteur
    Ds18b20.select(addr); // on charge son adresse
    Ds18b20.write(0x44, 1);        // start conversion, with parasite power on at the end
    phase = 2 ;  // la prochaine fois on ne passera pas par la
  }
  if ( phase == 2  && ( Temps - TempsMesurePrecedent ) > 800 )
  {
    TempsMesurePrecedent = Temps;
    phase = 0 ; // on recommencera !
    present = Ds18b20.reset ( ) ;
    Ds18b20.select ( addr ) ; // on selectionne le capteur suivant
    Ds18b20.write ( 0xBE ) ;         // Read Scratchpad
    for ( i = 0 ; i < 9 ; i++ )
    {
      data[i] = Ds18b20.read ( ) ; // on lit les donnees
    }
    mesure = ( data [ 1 ] << 8) | data [ 0 ] ;
    if ( type_s ) // suivant le nb de bits de resolution
    {
      mesure = mesure << 3; // 9 bit resolution default
      if ( data [ 7 ] == 0x10 )
      {
        mesure = (  mesure & 0xFFF0 ) + 12 - data [ 6 ] ;
      }
    }
    else
    {
      byte cfg = ( data [ 4 ] & 0x60 ) ;
      // at lower res, the low bits are undefined, so let's zero them
      if ( cfg == 0x00 ) mesure = mesure & ~7 ;  // 9 bit resolution, 93.75 ms
      else if ( cfg == 0x20 ) mesure = mesure & ~3 ; // 10 bit res, 187.5 ms
      else if ( cfg == 0x40 ) mesure = mesure & ~1 ; // 11 bit res, 375 ms
      //// default is 12 bit resolution, 750 ms conversion time
    }
    float mesureDallas = ( float ) mesure / 16.0 ;
    dallasFiltered[ noCapteur - 1 ].update(mesureDallas) ;
//    Dallas [ noCapteur - 1 ] = MesureDallas ;
//    Serial.println(String("DS n°")+noCapteur +":"+MesureDallas);
  }
}










void dump ( String nomFichier )
{
  Serial.println ( debutFichier ) ;
  File dataFile = SD.open ( nomFichier ) ;
  // if the file is available, write to it:
  if ( dataFile )
  {
    while ( dataFile.available ( ) )
    {
      Serial.write ( dataFile.read ( ) ) ;
    }
    dataFile.close ( ) ;
  }
  // if the file isn't open, pop up an error:
  else
  {
    Serial.println ( String ( "error opening" + nomFichier ) ) ;
  }
  Serial.println ( finFichier ) ;
}


void dumpRasp ( String nomFichier )
{
  Serial.println ( debutFichier ) ;
  File dataFile = SD.open ( nomFichier ) ;

  // if the file is available, write to it:
  if ( dataFile )
  {
    while ( dataFile.available ( ) )
    {
      Serial1.write ( dataFile.read ( ) ) ;
    }
    dataFile.close ( ) ;
  }
  // if the file isn't open, pop up an error:
  else
  {
    Serial1.println ( String ( "error opening" + nomFichier ) ) ;
  }
  Serial.println ( finFichier ) ;
}















void commandeSwitch ( int valeur )
{
  for ( byte boucle = 0 ; boucle < nbCommandeSwitch ; boucle ++ )
  {
    telecommande.send ( valeur , 24 ) ; // marche
  }
}

void affichageUsbSecondes ( void )
{

  Serial.print ( fmt1CmdMqtt (topicMesure, TrameMesures) ) ;
  /*
    Serial.print ( "Date = " ) ;
    Serial.println ( dateString ) ;
    Serial.print("Heure = ");
    Serial.println ( heureString ) ;
    Serial.print("Temperature int = ");
    Serial.println(temperatureInterieureEntiere);
    Serial.print("Humidite int = ");
    Serial.println(humiditeInterieureEntiere);
    Serial.print("Temperature ext = ");
    Serial.println(temperatureExterieureEntiere);
    Serial.print("Humidite ext = ");
    Serial.println(humiditeExterieureEntiere);
    Serial.println ( emission ) ;
    for ( byte boucle = nbCapteurs ; boucle > 0 ; boucle -- )
    {
    Serial.print( "Temperature Dallas N° " ) ;
    Serial.print ( boucle ) ;
    Serial.print ( " = " ) ;
    Serial.println ( String ( Dallas [ boucle - 1 ] ) ) ;
    }*/
}

void affichageSerieRaspSecondes ( void )
{
  Serial1.print ( fmt1CmdMqtt (topicMesure, TrameMesures) ) ;
  /*  Serial1.print ( "Date = " ) ;
    Serial1.println ( dateString ) ;
    Serial1.print("Heure = ");
    Serial1.println ( heureString ) ;
    Serial1.print("Temperature int = ");
    Serial1.println(temperatureInterieureEntiere);
    Serial1.print("Humidite int = ");
    Serial1.println(humiditeInterieureEntiere);
    Serial1.print("Temperature ext = ");
    Serial1.println(temperatureExterieureEntiere);
    Serial1.print("Humidite ext = ");
    Serial1.println(humiditeExterieureEntiere);
    Serial1.println ( emission ) ;
    for ( byte boucle = nbCapteurs ; boucle > 0 ; boucle -- )
    {
      Serial1.print( "Temperature Dallas N° " ) ;
      Serial1.print ( boucle ) ;
      Serial1.print ( " = " ) ;
      Serial1.println ( Dallas [ boucle - 1 ] ) ;
    }*/
}

void FonctionTexteTrameMesures ( void )
{
    String textDallas = "" ;

    for ( byte boucle = nbCapteurs ; boucle > 0 ; boucle -- )
    {
        textDallas = textDallas + separateurFichier +
                     String ( dallasFiltered[ boucle - 1 ].get() ) ;
    }
    TrameMesures = String ( dateString + separateurFichier +
                            heureString + separateurFichier +
                            temperatureInterieureEntiere + separateurFichier +
                            humiditeInterieureEntiere + separateurFichier +
                            temperatureExterieureEntiere + separateurFichier +
                            humiditeExterieureEntiere +
                            textDallas ) ;
}

String getTexteEnteteMesures ( void )
{
    return String("Date") +separateurFichier+ "Heure" +separateurFichier+
            "TempInt" +separateurFichier+ "HumidInt" +separateurFichier+
            "TempExt" +separateurFichier+ "HumidExt" +separateurFichier+
            "Temp3" +separateurFichier+ "Temp2"  +separateurFichier+ "Temp1" ;
}


void EnregistrementFichierMesure ( void )
{
  File dataFile = SD.open ( NomFichierMesure , FILE_WRITE ) ;
  if ( dataFile )
  {
    dataFile.println ( TrameMesures ) ;
    dataFile.close();
  }
  else
  {
    //erreur sur ecriture recurente carte SD
    erreur ( 9 ) ;
  }
}
