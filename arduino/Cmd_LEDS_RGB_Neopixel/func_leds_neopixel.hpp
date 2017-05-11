#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

void ledRedColorIntensity(uint8_t colorIntensity) {
  // For a set of NeoPixels the first NeoPixel is 0, second is 1, all the way up to the count of pixels minus one.
  for (int i = 0; i < NUMPIXELS; i++) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(colorIntensity, 0, 0)); // Moderately bright red color.
    pixels.show(); // This sends the updated pixel color to the hardware.
  }
}

void ledGreenColorIntensity(uint8_t colorIntensity) {
  // For a set of NeoPixels the first NeoPixel is 0, second is 1, all the way up to the count of pixels minus one.
  for (int i = 0; i < NUMPIXELS; i++) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0, colorIntensity, 0)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware.
  }
}

void ledBlueColorIntensity(uint8_t colorIntensity) {
  // For a set of NeoPixels the first NeoPixel is 0, second is 1, all the way up to the count of pixels minus one.
  for (int i = 0; i < NUMPIXELS; i++) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(0, 0, colorIntensity)); // Moderately bright blue color.
    pixels.show(); // This sends the updated pixel color to the hardware.
  }
}

void ledWhiteColorIntensity(uint8_t colorIntensity) {
  // For a set of NeoPixels the first NeoPixel is 0, second is 1, all the way up to the count of pixels minus one.
  for (int i = 0; i < NUMPIXELS; i++) {
    // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(colorIntensity, colorIntensity, colorIntensity)); // Moderately bright White color.
    pixels.show(); // This sends the updated pixel color to the hardware.
  }
}

// numMatrice8x8 from 1 to 4 and colorIntensity from 0 to 255
void ledHorticoleColorIntensity(uint8_t numMatrice8x8, uint8_t colorIntensity) {
//  uint8_t colorIntensity = map(analogRead(A0), 0, 1023, 0, 255);

  /* Update 1 matrice of 64 LEDs */
  // 2x 8 Red
  for (int i = 0; i < 16; i++) {
    pixels.setPixelColor(i+(64*(numMatrice8x8-1)), pixels.Color(colorIntensity, 0, 0)); // Moderately bright green color.
  }
  // 1x 8 Blue
  for (int i = 16; i < 24; i++) {
    pixels.setPixelColor(i+(64*(numMatrice8x8-1)), pixels.Color(0, 0, colorIntensity)); // Moderately bright green color.
  }
  // 2x 8 Red
  for (int i = 24; i < 40; i++) {
    pixels.setPixelColor(i+(64*(numMatrice8x8-1)), pixels.Color(colorIntensity, 0, 0)); // Moderately bright green color.
  }
  // 1x 8 Blue
  for (int i = 40; i < 48; i++) {
    pixels.setPixelColor(i+(64*(numMatrice8x8-1)), pixels.Color(0, 0, colorIntensity)); // Moderately bright green color.
  }
  // 2x 8 Red
  for (int i = 48; i < 64; i++) {
    pixels.setPixelColor(i+(64*(numMatrice8x8-1)), pixels.Color(colorIntensity, 0, 0)); // Moderately bright green color.
  }
  // This sends the updated pixel color to the hardware.
  pixels.show();
}


