// This header contains al the RGB interaces for the main LEDs of the clock
#include <FastLED.h>

// Configure the LED strip with these constants
constexpr int kLEDPin = 0;
constexpr int kNumLEDs = 6;

class LEDController {
public:
  LEDController() {
    FastLED.addLeds<WS2812B, kLEDPin, GRB>(leds, kNumLEDs);
    FastLED.setBrightness(64);
  }

  // Set the whole LED strip to be a solid color and display it.
  void setSolid(CRGB color) {
    fill_solid(leds, kNumLEDs, color);
    show();
  }

  // Latch any new values into the LEDs
  void show() const {
    FastLED.show();
  }

private:
  CRGB leds[kNumLEDs];
};
