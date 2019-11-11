// This header contains al the RGB interaces for the main LEDs of the clock
#include <FastLED.h>

// Configure the LED strip with these constants
constexpr int kLEDPin = 0;
constexpr int kNumLEDs = 36;

class LEDController {
public:
  LEDController() {
    FastLED.addLeds<WS2812B, kLEDPin, GRB>(_leds, kNumLEDs);
    FastLED.setBrightness(10);
  }

  // Set the whole LED strip to be a solid color and display it.
  void setSolid(CRGB color) {
    fill_solid(_leds, kNumLEDs, color);
    show();
  }

  // Fill in a rainbow on the LED strip
  void setRainbow(uint8_t offset) {
    fill_rainbow(_leds, kNumLEDs, offset, 0xFF / kNumLEDs);
    show();
  }

  // Latch any new values into the LEDs
  void show() const {
    FastLED.show();
  }

  CRGB _leds[kNumLEDs];
};
