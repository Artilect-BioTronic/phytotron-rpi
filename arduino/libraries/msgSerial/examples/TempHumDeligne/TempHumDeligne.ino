

// Bibliothéques
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Deuligne.h>
#include "Adafruit_HTU21DF.h"
//#include <DHT.h>
//#include <DHT_U.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <SPI.h>
#include <SD.h>
#include <RCSwitch.h>


//reglages
const unsigned long intervalleEnregistrement = 60000 ;
const unsigned long intervalleAffichage = 1000 ;
const byte nbCommandeSwitch = 10 ;
const int humiditicateurMarche = 5393 ;
const int humiditicateurArret = 5396 ;
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
byte plageTemp = 2 ;
byte consigneHum = 50 ;
byte plageHum = 0 ;
bool commandeChauffage = false ;
bool commandeRefroidissement = false ;
bool commandeHum = false ;
unsigned long positionFichierConsignes ;

//enregistrement SD sur Memoire
//const int chipSelect = 10 ; // uno
const int chipSelect = 53 ; // mega
//noms des fichiers
const String  NomFichierConsignes  = "cons.csv" ;
const String  NomFichierMesure  = "mes.csv" ;
const String  NomFichierCommandes  = "com.csv" ;
const unsigned long  positionEnregistrementFichierConsignes  = 7 ; // position avant derniere consigne temperature
const String separateurFichier  = ";" ;

//Capteurs
#define DHTPIN  2 // pin capteur humidite
// Uncomment the type of sensor in use:
//#define DHTTYPE           DHT11     // DHT 11
//#define DHTTYPE           DHT22     // DHT 22 (AM2302)
//#define DHTTYPE           DHT21     // DHT 21 (AM2301)
//DHT_Unified dht( DHTPIN , DHTTYPE ) ;
Adafruit_HTU21DF htu = Adafruit_HTU21DF();


//Prises telecommandees
RCSwitch telecommande = RCSwitch ( ) ;
const int pinTelecommande = 7 ;

// Variables pour temps et mesures
byte temperatureEntiere ;
byte humiditeEntiere ;
String heuresDix ;
String minutesDix ;
String secondesDix ;
String heureString ;
String dateString ;
unsigned long tempsEnregistrement = 0 ;
unsigned long tempsAffichage = 0 ;

// Variables pour communication serie
char lecturePortSerie ;
String messageRecu = "" ;

void setup(void)
{
  Serial.begin ( 115200 ) ;
  while ( !Serial )
  {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  lcd.init ( ) ;
  lcd.createChar ( 0 , degres ) ;
  //  dht.begin ( ) ;
  releveRTC ( ) ; //on releve date et heure sur l'horloge RTC
  sensor_t sensor; //Initialise the sensor
  //  dht.temperature().getSensor(&sensor);
  //  dht.humidity().getSensor(&sensor);
  if (!htu.begin()) {
    Serial.println("Couldn't find sensor!");
    while (1);
  }


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
    regFileEcriture.close();
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



  releveValeurs ( ) ;
  afficheLCDregulier ( ) ;
  tmElements_t tm ;
  RTC.read ( tm ) ;

  while ( tm.Second != 0 )  //Pour partir à une Minute entiére
  {
    RTC.read ( tm ) ;
    delay ( 100 ) ;

    // affichage regulier
    if ( ( ( millis ( ) - tempsAffichage > intervalleAffichage ) ) && ! menu )
    {
      tempsAffichage = tempsAffichage + intervalleAffichage ;
      releveValeurs ( ) ;
      afficheLCDregulier ( ) ;
    }

  }
  tempsEnregistrement = tempsAffichage = millis ( ) ;

  /*
    while ( tempsEnregistrement > intervalleEnregistrement )
    {
      tempsEnregistrement = tempsEnregistrement - intervalleEnregistrement ;
    }

    while ( tempsAffichage > intervalleAffichage )
    {
      tempsAffichage = tempsAffichage - intervalleAffichage ;
    }
  */
  releveValeurs ( ) ;
  afficheLCDregulier ( ) ;
  telecommande.enableTransmit ( pinTelecommande ) ;
  commandeSwitch ( humiditicateurArret ) ;
  commandeSwitch ( refroidissementArret ) ;
  commandeSwitch ( chauffageArret ) ;
}


void loop ( )
{
  // enregistrement regulier
  if ( millis ( ) - tempsEnregistrement > intervalleEnregistrement )
  {
    //preparation du prochain enregistrement
    tempsEnregistrement = tempsEnregistrement + intervalleEnregistrement ;
    releveValeurs ( ) ;
    File dataFile = SD.open ( NomFichierMesure , FILE_WRITE ) ;
    if ( dataFile )
    {
      dataFile.print ( dateString ) ;
      dataFile.print ( separateurFichier ) ;
      dataFile.print ( heureString ) ;
      dataFile.print ( separateurFichier ) ;
      //     dataFile.print ( numeroDix ( temperatureEntiere ) ) ;
      dataFile.print ( temperatureEntiere ) ;
      dataFile.print ( separateurFichier ) ;
      //      dataFile.print ( numeroDix ( humiditeEntiere ) ) ;
      dataFile.print ( humiditeEntiere ) ;
      dataFile.println ( "" ) ;
      dataFile.close();
    }
    else
    {
      //erreur sur ecriture recurente carte SD
      erreur ( 9 ) ;
    }
  }

  // affichage regulier
  if ( ( ( millis ( ) - tempsAffichage > intervalleAffichage ) ) && ! menu )
  {
    tempsAffichage = tempsAffichage + intervalleAffichage ;
    releveValeurs ( ) ;
    afficheLCDregulier ( ) ;
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
        consigneTemp ++ ;
        lcd.setCursor ( 0 , 1 ) ;
        lcd.print ( numeroDix ( consigneTemp ) ) ;
      }
      if ( reglageHum )
      {
        consigneHum ++ ;
        lcd.setCursor ( 13 , 1 ) ;
        lcd.print ( numeroDix ( consigneHum ) ) ;
      }
      selection = false ;
    }

    if ( 487 < valeurStick && valeurStick < 504 && selection  ) // bas
    {
      if ( reglageTemp )
      {
        consigneTemp -- ;
        lcd.setCursor ( 0 , 1 ) ;
        lcd.print ( numeroDix ( consigneTemp ) ) ;
      }

      if ( reglageHum )
      {
        consigneHum -- ;
        lcd.setCursor ( 13 , 1 ) ;
        lcd.print ( numeroDix ( consigneHum ) ) ;
      }
      selection = false ;
    }

    if ( 850 < valeurStick && valeurStick < 870 && selection && ! validation ) // appuye
    {
      //enregistre les consignes
      validation = true ;
      //Serial.println ( "appuye" ) ;
    }
  }

  if ( menu && ! selection && valeurStick > 1013 )
  {
    selection = true ;
    //Serial.println ( "selection" ) ;
  }

  if ( menu && selection && validation && valeurStick > 1013 )
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

    File regFile = SD.open ( NomFichierConsignes , FILE_WRITE ) ;
    if ( regFile )
    {
      regFile.print ( dateString ) ;
      regFile.print ( separateurFichier ) ;
      regFile.print ( heureString ) ;
      regFile.print ( separateurFichier ) ;
      regFile.print ( numeroDix ( consigneTemp ) ) ;
      regFile.print ( separateurFichier ) ;
      regFile.print ( numeroDix ( consigneHum ) ) ;
      regFile.println ( "" ) ;
      regFile.close();
    }
    else
    {
      //non ecriture fichier consigne sur carte SD
      erreur ( 10 ) ;
    }
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
  if ( humiditeEntiere < ( consigneHum - plageHum ) & ! commandeHum )
  {
    //telecommande.send ( 5393 , 24 ) ; // marche
    commandeSwitch ( humiditicateurMarche ) ;
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
  if ( humiditeEntiere > consigneHum & commandeHum )
  {
    //telecommande.send ( 5396 , 24 ) ; // arret
    commandeSwitch ( humiditicateurArret ) ;
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
  if ( temperatureEntiere < ( consigneTemp - ( plageTemp * 2 ) ) && ! commandeChauffage )
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
  if ( temperatureEntiere > ( consigneTemp - plageTemp ) && commandeChauffage )
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
  if ( temperatureEntiere > ( consigneTemp + ( plageTemp * 2 ) ) && ! commandeRefroidissement )
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
  if ( temperatureEntiere < ( consigneTemp + plageTemp ) && commandeRefroidissement )
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






  // Lecture port serie

  if ( Serial.available ( ) > 0 )
  {
    while ( Serial.available ( ) > 0)
    {
      lecturePortSerie = Serial.read ( ) ;
      messageRecu = String ( messageRecu + lecturePortSerie ) ;
    }
    Serial.print ( "messageRecu =>" ) ;   // send an initial string
    Serial.print ( messageRecu ) ;   // send an initial string
    Serial.println ( "<=" ) ;   // send an initial string
  }
  if ( messageRecu == "cons" )
  {
    dump ( NomFichierConsignes ) ;
  }
  if ( messageRecu == "mes" )
  {
    dump ( NomFichierMesure ) ;
  }
  if ( messageRecu == "com" )
  {
    dump ( NomFichierCommandes ) ;
  }
  messageRecu = "" ;
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
    heureString = String ( numeroDix ( tm.Hour )
                           + ":"
                           + numeroDix ( tm.Minute )
                           + ":"
                           + numeroDix ( tm.Second ) ) ;

    dateString = String ( numeroDix ( tm.Day )
                          + "/"
                          + numeroDix ( tm.Month )
                          + "/"
                          + tmYearToCalendar ( tm.Year ) ) ;

  }
}

//fonction d'affichage LCD
void afficheLCD ( String texte , unsigned int positionColonne , boolean ligne  )
{
  lcd.setCursor ( positionColonne , ligne ) ;
  lcd.print ( texte ) ;
}

void releveValeurs ( void )
{
  releveRTC ( ) ; //on releve date et heure sur l'horloge RTC
  sensors_event_t event; // Lancer les mesures
  /*
    dht.temperature().getEvent(&event); //temperature par DHT11
    if (isnan(event.temperature))
    {
    erreur ( 10 ) ;
    }
    else
    {
    temperatureEntiere = event.temperature ;
    }
    dht.humidity ( ) . getEvent ( &event ) ; //humidite par DHT11
    if ( isnan( event . relative_humidity ) )
    {
    erreur ( 11 ) ; ;
    }
    else
    {
    humiditeEntiere = event.relative_humidity ;
    }
  */
  temperatureEntiere = int ( htu.readTemperature ( ) ) ;
  humiditeEntiere = int ( htu.readHumidity ( ) ) ;

}

void afficheLCDregulier ( void )
{
  afficheLCD ( numeroDix ( temperatureEntiere ) , 0 , 0 ) ;
  lcd.setCursor ( 2 , 0 ) ;
  lcd.write ( 0 ) ;
  afficheLCD ( "C" , 3 , 0 ) ;
  afficheLCD ( dateString , 6 , 0 ) ;
  afficheLCD ( numeroDix ( humiditeEntiere ) , 0 , 1 ) ;
  afficheLCD ( "%" , 2 , 1 ) ;
  afficheLCD ( heureString , 8 , 1 ) ;
}


void dump ( String nomFichier )
{
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
}

void commandeSwitch ( int valeur )
{
  for ( byte boucle = 0 ; boucle < nbCommandeSwitch ; boucle ++ )
  {
    telecommande.send ( valeur , 24 ) ; // marche
  }
}
