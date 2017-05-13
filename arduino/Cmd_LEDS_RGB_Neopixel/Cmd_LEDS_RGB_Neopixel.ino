/*
  Programme sur Arduino Nano
  pour projet Phytotron
  contact: arnauld.biganzoli@gmail.com

  === Commande afficheur 4x matrice NeoPixel ===
  https://fr.aliexpress.com/w/wholesale-8x8-ws2812.html?spm=2114.06010108.0.0.aJirw2&initiative_id=SB_20170510120550&site=fra&groupsort=1&SortType=price_asc&g=y&SearchText=8x8+ws2812

  === Text scroller on 7 segment 8 digits display ===
  with http://tiptopboards.com/106-module-8-chiffres-7-segments-8-boutons-8-led-bicolores.html

  Use Library examples for TM1638.
  http://tiptopboards.free.fr/arduino_forum/viewtopic.php?f=2&t=17
  http://tronixstuff.com/2012/03/11/arduino-and-tm1638-led-display-modules/
  Copyright (C) 2011 Ricardo Batista <rjbatista at gmail dot com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the version 3 GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "Arduino.h"

#include <TM1638.h>
#include <Adafruit_NeoPixel.h>
#include <avr/power.h>

//#include "msgSerial.h"

// define a module on data pin 8, clock pin 9 and strobe pin 7 +
TM1638 module(8, 9, 7);
char message[] = "        ArtiLEct-biotronic        ";
uint8_t thisPosition = 0;

// Which pin on the Arduino is connected to the NeoPixels?
#define NEOPIXEL_PIN   6

// How many NeoPixels are attached to the Arduino?
#define NUMBER_OF_MATRICE8x8   4
#define NUMPIXELS              64*NUMBER_OF_MATRICE8x8

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

#include "func_leds_neopixel.hpp"

// Globale variable
uint8_t ledIntensity = 150;        // from 0 to 255 (min to max)
byte receivedMode='1'; // to store the data received from UART
uint16_t scrollingTextDelay = 500; // from 100 to 1300 (min to max)

int chgIntensity(const String& aStr);
int chgLightMode(const String& aStr);
const String sListLightMode = "01rgbwRGBW";  // list of possible light modes

size_t msgSErrorCmd(const String& aMsg)   {
    return Serial.print(String("CM+")+ aMsg+ '\n');
}


void setup() {
  Serial.begin(9600); // initialize the serial communication
  pixels.begin();     // initialize the NeoPixel library

  // I fill info on sketch
//  sketchInfo.setFileDateTime(F(__FILE__), F(__DATE__), F(__TIME__));
//  // I send identification of sketch
//  sendSketchId("");
//  sendSketchBuild("");
  msgSErrorCmd(String("idSketch:Cmd_Short_LEDS_RGB_Neopixel") );
}

void loop() {

    char charReceived = '\0';

  // Check if data has been sent from the computer:
  if (Serial.available()) {
    // read the most recent byte (which will be from 0 to 255):
    charReceived = Serial.read();
    Serial.print("received:"); Serial.println(charReceived);

    // if incoming char is  appropriate for mode
    if ( sListLightMode.indexOf(charReceived) != -1 )   {
        receivedMode = charReceived;
        msgSErrorCmd(String("mode/OK:")+ charReceived);
    }
    else {
        // it is used for intensity
        ledIntensity = charReceived;
        msgSErrorCmd(String("intensity/OK:")+ String(int(ledIntensity)));
    }
  }


  switch (receivedMode) {
    case '0':
      //do something when var equals 0
      ledRedColorIntensity(0);
      break;

    case '1':
      //do something when var equals 1
      ledHorticoleColorIntensity(1, ledIntensity); // Panel 1
      ledHorticoleColorIntensity(2, ledIntensity); // Panel 2
      ledHorticoleColorIntensity(3, ledIntensity); // Panel 3
      ledHorticoleColorIntensity(4, ledIntensity); // Panel 4
      break;

    case 'r':
    case 'R':
      //do something when var equals 2
      ledRedColorIntensity(ledIntensity);
      break;

    case 'g':
    case 'G':
      //do something when var equals 2
      ledGreenColorIntensity(ledIntensity);
      break;

    case 'b':
    case 'B':
      //do something when var equals 2
      ledBlueColorIntensity(ledIntensity);
      break;

    case 'w':
    case 'W':
      //do something when var equals 2
      ledWhiteColorIntensity(ledIntensity);
      break;

  case '\0':
    //do something when var equals 2
    ledWhiteColorIntensity(ledIntensity);
    break;

    default:
      // if nothing else matches, do the default
      // default is optional
      // ledGreenColorIntensity(10); // make green by default --> optional
      charReceived = '\0';
      break;
  }


  // Text scroller on 7 segment 8 digits display
  static long lastTime=0;
  if (millis()-lastTime > scrollingTextDelay)   {
      thisPosition++;
      lastTime = millis();
  }
  if (thisPosition > 22) {
    thisPosition = 0;
  }
  module.setDisplayToString(message + thisPosition, 0);

  // For scrolling quickly or slowly
  //  delay(map(analogRead(A0), 0, 1023, 100, 1300)); // for scrolling quickly or slowly
//  delay(scrollingTextDelay);
}

