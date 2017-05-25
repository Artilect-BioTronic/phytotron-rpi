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
#include <SD.h>
#include <RCSwitch.h>
#include <OneWire.h>

#define INSTAL_CPLT     1
#define INSTAL_RTC      2

#define INSTALLATION  INSTAL_CPLT

#include "msgSerial.h"
#include "TempHumMsg.h"


// Si vous n avez pas de shield deuligne, il faut connecter pinStick a 5V,
//   sinon le sketch voit des actions permanentes pour naviguer dans le menu
// Si vous n utilisez pas le serial1, il faut bouchonner rx

// example of frame:
// -t "/phytotron/cli/mesure/D;H;ti;hi;te;he;t3;t2;t1"
// -m "2017-05-08;12:34:56;11;12.1;21.1;22.1;33.1;32.1;31.1"
const String topicMesure        = "mesure/D;H;ti;hi;te;he;t3;t2;t1";
const String topicConsigneState = "csgn/state/D;H;t;h";
const String head1="CM+";
const String endOL="\n";

String fmt1CmdMqtt(const String & aTopic, const String& aLoad)
{
    return head1 + aTopic + ":" + aLoad + endOL;
}

#if (INSTALLATION == INSTAL_CPLT)
const long serial1Rate=   115200;
const long serialRate=    38400;
bool bDemarreMnEntiere = true;
#else
const long serial1Rate=   9600;
const long serialRate=    38400;
bool bDemarreMnEntiere = false;
#endif

//reglages
const byte intervalleEnregistrement = 60 ; // en secondes
const byte intervalleAffichage = 1 ; // en secondes
const byte nbCommandeSwitch = 5 ;
const int humidificateurMarche = 5393 ;
const int humidificateurArret = 5396 ;
const int refroidissementMarche = 4433 ;
const int refroidissementArret = 4436 ;
const int chauffageMarche = 1361 ;
const int chauffageArret = 1364 ;


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
const int pinStick = A0 ; // stick du deuligne pour menu
// variables et constantes pour menu
const unsigned long delaisMenu = 5000 ;
unsigned int valeurStick = 0 ;
boolean menu = false ;
boolean selection = false ;
boolean validation = false ;
unsigned long debutMenu = 0 ;
boolean reglageTemp = false ;
boolean reglageHum = false ;
byte consigneTemp = 20 ;
byte consigneTempProvisoire = 20 ;
byte plageTemp = 2 ;
byte consigneHum = 50 ;
byte consigneHumProvisoire = 50 ;
byte plageHum = 0 ;
bool commandeChauffage = false ;
bool commandeRefroidissement = false ;
bool commandeHum = false ;
unsigned long positionFichierConsignes ;


//Génération des trames
String TrameMesures = "" ;
const char pointVirgule = ';' ;

//enregistrement SD sur Memoire
//const int chipSelect = 10 ; // uno
const int chipSelect = 53 ; // mega
//noms des fichiers
const String  NomFichierConsignes  = "cons.csv" ;
const String  NomFichierMesure  = "mes.csv" ;
const String  NomFichierCommandes  = "com.csv" ;
const unsigned long  positionEnregistrementFichierConsignes  = 7 ; // position avant derniere consigne temperature
const String separateurFichier  = ";" ;

//Echanges de fichiers
const String debutFichier = "Debut du fichier" ;
const String finFichier = "Fin du fichier" ;
const String  commandeLectureFichierConsignes  = "cons" ;
const String  commandeLectureFichierMesure  = "mes" ;
const String  commandeLectureFichierCommandes  = "com" ;



//Capteurs temperature-humidite
//Exterieur
#define CapteurHumiditeTemperatureExterieurPIN  2 // pin capteur humidite
// Uncomment the type of sensor in use:
#define CapteurHumiditeTemperatureExterieurTYPE           DHT11     // DHT 11
//#define CapteurHumiditeTemperatureExterieurTYPE           DHT22     // DHT 22 (AM2302)
//#define CapteurHumiditeTemperatureExterieurTYPE           DHT21     // DHT 21 (AM2301)
DHT_Unified CapteurHumiditeTemperatureExterieur( CapteurHumiditeTemperatureExterieurPIN , CapteurHumiditeTemperatureExterieurTYPE ) ;


//Interieur
#define CapteurHumiditeTemperatureInterieurPIN  40 // pin capteur humidite
// Uncomment the type of sensor in use:
//#define CapteurHumiditeTemperatureInterieurTYPE           DHT11     // DHT 11
#define CapteurHumiditeTemperatureInterieurTYPE           DHT22     // DHT 22 (AM2302)
//#define CapteurHumiditeTemperatureInterieurTYPE           DHT21     // DHT 21 (AM2301)
DHT_Unified CapteurHumiditeTemperatureInterieur( CapteurHumiditeTemperatureInterieurPIN , CapteurHumiditeTemperatureInterieurTYPE ) ;





//Adafruit_HTU21DF htu = Adafruit_HTU21DF();
//boolean HTU21 = false ; // présence capteur HTU21

//capteur(s) temperature Dallas
const byte nbCapteurs = 3 ;
const int PinOneWire = 69 ; // pin capteur temperature Dallas
byte octet ;
byte present = 0;
byte type_s;
byte data[12];
byte addr[8];
float celsius ;
String TempDs18b20 = "" ;
String moment = "" ;
String emission = "" ; // pour envoyer le message au Kikiwi
String emissionEnCours = "" ; // pour préparer la trame suivante
byte noCapteur = 0 ; // pour éventuellement compter le nombre de capteurs
byte i;
byte phase = 0 ;
int mesure = 0 ;
unsigned long Temps = 0 ;
unsigned long TempsMesure = 0 ;
unsigned long TempsEnregistrement = 0 ;
unsigned long TempsMesurePrecedent = 0 ;
unsigned long TempsEnregistrementPrecedent = 0 ;
float Dallas [ nbCapteurs ] ;



//Prises telecommandees
RCSwitch telecommande = RCSwitch ( ) ;
const int pinTelecommande = 7 ;
OneWire  Ds18b20 ( PinOneWire ) ;

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
byte minutesEnregistrementPrecedent = 0 ;
byte minutes = 0 ;
byte secondesAffichagePrecedent = 0 ;
byte secondes = 0 ;
unsigned long tempsAffichage = 0 ;

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
  commandeSwitch ( chauffageArret ) ;
  Serial1.begin ( serial1Rate ) ;
  Serial.begin ( serialRate ) ;
  while ( !Serial )
  {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.println ( "Debut de l'init" ) ;

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
      dataFile.print ( "Date" ) ;
      dataFile.print ( separateurFichier ) ;
      dataFile.print ( "Heure" ) ;
      dataFile.print ( separateurFichier ) ;
      dataFile.print ( "Temperature" ) ;
      dataFile.print ( separateurFichier ) ;
      dataFile.print ( "Humidite" ) ;
      dataFile.println ( "" ) ;
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
  tmElements_t tm ;
  secondesAffichagePrecedent = secondes ;
  Serial.println ( "Pas de panique, PAS BESOIN d attendre la fin de la minute entiere dans setup" ) ;
  do
  {
    releveRTC ( ) ; //on releve date et heure sur l'horloge RTC
    // affichage regulier
    if ( ( secondes != secondesAffichagePrecedent ) && ! menu )
    {
      secondesAffichagePrecedent = secondes ;
      //tempsAffichage = tempsAffichage + intervalleAffichage ;
      releveValeurs ( ) ;
      afficheLCDregulier ( ) ;
      Serial.println ( 61 - secondes ) ;
    }
  }
  while ( secondes != 0 && bDemarreMnEntiere ) ; //Pour partir à une Minute entiére
  tempsEnregistrement = tempsAffichage = millis ( ) ;
  minutesEnregistrementPrecedent = minutes ;
  secondesAffichagePrecedent = secondes ;
}


void loop ( )
{
  Temps = millis ( ) ;
  releveRTC ( ) ; //on releve date et heure sur l'horloge RTC
  mesureTemperature18B20 ( ) ;

  // enregistrement regulier
//  if ( minutes != minutesEnregistrementPrecedent )
  if ( (millis() - tempsEnregistrement)/1000 > intervalleEnregistrement)
  {
      tempsEnregistrement = millis();
      minutesEnregistrementPrecedent = minutes ;
      EnregistrementFichierMesure ( ) ;

      // we regularly re apply consignes to switches
      if (commandeRefroidissement)
          commandeSwitch ( refroidissementMarche ) ;
      else
          commandeSwitch ( refroidissementArret ) ;

      if (commandeChauffage)
          commandeSwitch ( chauffageMarche) ;
      else
          commandeSwitch ( chauffageArret) ;

      if (commandeHum)
          commandeSwitch ( humidificateurMarche) ;
      else
          commandeSwitch ( humidificateurArret) ;
  }

  // affichage regulier
  //if ( ( ( millis ( ) - tempsAffichage > intervalleAffichage ) ) && ! menu )
  if ( ( secondes !=  secondesAffichagePrecedent ) && ! menu )
  {
    //tempsAffichage = tempsAffichage + intervalleAffichage ;
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

  // Asservissements
  // Humidite prise " D"
  if ( (humiditeInterieureEntiere < ( consigneHum - plageHum )) & ! commandeHum )
  {
    //telecommande.send ( 5393 , 24 ) ; // marche
    commandeSwitch ( humidificateurMarche ) ;
    commandeHum = true ;
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

  // Chauffage prise " A "
  if ( temperatureInterieureEntiere < ( consigneTemp - ( plageTemp * 2 ) ) && ! commandeChauffage )
  {
    //telecommande.send ( 1361 , 24 ) ; // marche
    commandeSwitch ( chauffageMarche ) ;
    commandeChauffage = true ;
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
  if ( temperatureInterieureEntiere > ( consigneTemp - plageTemp ) && commandeChauffage )
  {
    //telecommande.send ( 1364 , 24 ) ; // arret
    commandeSwitch ( chauffageArret ) ;
    commandeChauffage = false ;
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

  // Refroidissement prise " B "
  if ( temperatureInterieureEntiere > ( consigneTemp + ( plageTemp * 2 ) ) && ! commandeRefroidissement )
  {
    //telecommande.send ( 4433 , 24 ) ; // marche
    commandeSwitch ( refroidissementMarche ) ;
    commandeRefroidissement = true ;
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
  if ( temperatureInterieureEntiere < ( consigneTemp + plageTemp ) && commandeRefroidissement )
  {
    //telecommande.send ( 4436 , 24 ) ; // arret
    commandeSwitch ( refroidissementArret ) ;
    commandeRefroidissement = false ;
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

  lectureSerialUSB_PM();

  // Lecture port serie RasbPi
  lectureSerialRaspi_PM() ;

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

String getTrameConsigne() {
    String sTrame = "";
    sTrame = dateString   +separateurFichier+
             heureString  +separateurFichier+
             numeroDix ( consigneTemp )  +separateurFichier+
             numeroDix ( consigneHum );
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
  if ( phase == 0 && ( Temps - TempsMesurePrecedent ) > 150  ) // a chaque nouveau capteur et apres un certain temps
  {
    TempsMesurePrecedent += 150 ; // on lance le chrono
    if ( !Ds18b20.search(addr)) // est-ce que le dernier est passé ?
    {
      noCapteur = 0 ; // on recommence au debut
      Ds18b20.reset_search ( ) ; // on demande l'identification
      //delay ( 10 ) ;
      emission = emissionEnCours ; // on met a jour le texte qui va etre emis
      emissionEnCours = "" ; // on efface la variable en cours puisque on est au debut
    }
    else // il y a un capteur a mesurer
    {
      phase = 1 ; // au coup suivant on ne passeras pas par la
      if ( noCapteur > 0 ) // si autre que le premier
      {
        emissionEnCours += ";" ; //  mettre un point virgule !
      }
      noCapteur ++ ; // un de plus
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
    TempsMesurePrecedent += 800 ;
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
    float MesureDallas = ( float ) mesure / 16.0 ;
    Dallas [ noCapteur - 1 ] = MesureDallas ;
    emissionEnCours += String ( ( float ) mesure / 16.0 ) ; // on ecris la nouvelle valeur sur la chaine
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
  static float oldDallas[nbCapteurs]={15., 15., 15.};

  for ( byte boucle = nbCapteurs ; boucle > 0 ; boucle -- )
  {
      String sDallas = String(Dallas [ boucle - 1 ]);
      //  si la valeur mesurée est bidon avec 85.00, on remet la valeur precedente
      if (sDallas.equals("85.00"))
          textDallas = textDallas + String ( pointVirgule + String ( oldDallas [ boucle - 1 ] ) ) ;
      else   {
          textDallas = textDallas + String ( pointVirgule + String ( Dallas [ boucle - 1 ] ) ) ;
          oldDallas [ boucle - 1 ] = Dallas [ boucle - 1 ];
      }
  }
  TrameMesures = String ( dateString + pointVirgule +
                          heureString + pointVirgule +
                          temperatureInterieureEntiere + pointVirgule +
                          humiditeInterieureEntiere + pointVirgule +
                          temperatureExterieureEntiere + pointVirgule +
                          humiditeExterieureEntiere +
                          textDallas ) ;
}


void EnregistrementFichierMesure ( void )
{
  File dataFile = SD.open ( NomFichierMesure , FILE_WRITE ) ;
  if ( dataFile )
  {
    dataFile.println ( TrameMesures ) ;
    /*dataFile.print ( dateString ) ;
      dataFile.print ( separateurFichier ) ;
      dataFile.print ( heureString ) ;
      dataFile.print ( separateurFichier ) ;
      //     dataFile.print ( numeroDix ( temperatureInterieureEntiere ) ) ;
      dataFile.print ( temperatureInterieureEntiere ) ;
      dataFile.print ( separateurFichier ) ;
      //      dataFile.print ( numeroDix ( humiditeInterieureEntiere ) ) ;
      dataFile.print ( humiditeInterieureEntiere ) ;
      dataFile.println ( "" ) ;*/
    dataFile.close();
  }
  else
  {
    //erreur sur ecriture recurente carte SD
    erreur ( 9 ) ;
  }
}
