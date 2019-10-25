// This header contains al the RGB interaces for the main LEDs of the clock

#include <FastLED.h>

#define NUM_LEDS 6
#define DATA_PIN 0

static CRGB leds[NUM_LEDS];

// Setup FastLED to control the main RGB LED strip
void leds_setup() {
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  FastLED.setBrightness(64);
}

void leds_setSolid(CRGB color) {
  fill_solid(leds, NUM_LEDS, color);
  FastLED.show();
} 
