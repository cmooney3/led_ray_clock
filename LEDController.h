// This header contains al the RGB interaces for the main LEDs of the clock
#include <FastLED.h>

// Configure the LED strip with these constants
constexpr int kLEDPin = 0;
constexpr int kNumLEDs = 36;

// How many notches "off" from noon is LED 0
constexpr uint8_t kClockRotationalOffsetToNoon = 1;

class LEDController {
public:
  void setup() {
    FastLED.addLeds<WS2812B, kLEDPin, GRB>(_leds, kNumLEDs);
    FastLED.setBrightness(32);
  }

  // Set the whole LED strip to be a solid color and display it.
  void fillSolid(CRGB color) {
    fill_solid(_leds, kNumLEDs, color);
  }

  void setSingleLEDColor(uint8_t position, CRGB color) {
    _leds[position] = color;
  }

  // Set the color for the markers at the poles: noon, three, six, and nine
  void setPoleMarkers() {
    // The color used to mark the "poles"
    static const CRGB kMarkerColor = CRGB::White;

    // Precompute positions of the "poles" at noon, 3, 6, and 9
    constexpr static uint8_t kPoleIndicies[4] = {
      (0 + kClockRotationalOffsetToNoon) % kNumLEDs,               // Noon
      (kNumLEDs / 4 + kClockRotationalOffsetToNoon) % kNumLEDs,    // Three
      (kNumLEDs / 2 + kClockRotationalOffsetToNoon) % kNumLEDs,    // Six
      (3 * kNumLEDs / 4 + kClockRotationalOffsetToNoon) % kNumLEDs // Nine
    };

    // Set each Pole LED to the pole color
    for (uint8_t pole = 0; pole < sizeof(kPoleIndicies); pole++) {
      setSingleLEDColor(kPoleIndicies[pole], kMarkerColor);
    }
  }

  // Latch any new values into the LEDs
  void show() const {
    FastLED.show();
  }

  CRGB _leds[kNumLEDs];
};
