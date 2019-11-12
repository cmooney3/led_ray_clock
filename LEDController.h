// This header contains al the RGB interaces for the main LEDs of the clock
#include <FastLED.h>

// This header includes the declarations of the time structures
#include <RtcDS3231.h>

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

  // Set a single LED to a specific color in "modulo coordinates"
  // This offets the position to be reletive to the "noon" position, not the
  // actual start of the LED strip.
  // eg: 0 is noon
  //     1 is one LED clockwise of noon (twards 1 o'clock)
  //    -1 is one LED counter-clockwise of noon (twards 11 o'clock)
  void setSingleLEDColor(int16_t position, CRGB color) {
    position = (kNumLEDs - position + kClockRotationalOffsetToNoon) % kNumLEDs;
    _leds[position] = color;
  }

  // Set the color for the markers at the poles: noon, three, six, and nine
  void setPoleMarkers() {
    // The color used to mark the "poles"
    static const CRGB kMarkerColor = CRGB::White;

    // Precompute positions of the "poles" at noon, 3, 6, and 9
    constexpr static uint8_t kPoleIndicies[4] = {
      0,                 // Noon
      kNumLEDs / 4,      // Three
      kNumLEDs / 2,      // Six
      (3 * kNumLEDs) / 4 // Nine
    };

    // Set each Pole LED to the pole color
    for (uint8_t pole = 0; pole < sizeof(kPoleIndicies); pole++) {
      setSingleLEDColor(kPoleIndicies[pole], kMarkerColor);
    }
  }

  // Display a given time on the clock face.
  // TODO: Add a color picker as an additional parameter
  void displayTime(const RtcDateTime& time) {
    uint16_t second_led = time.Second() * kNumLEDs / 60;
    setSingleLEDColor(second_led, CRGB::Red);

    uint16_t minute_led = time.Minute() * kNumLEDs / 60;
    setSingleLEDColor(minute_led, CRGB::Green);

    uint16_t hour_led = (time.Hour() % 12) * kNumLEDs / 12;
    hour_led += (kNumLEDs * time.Minute()) / (12 * 60); // Advance within the hour based on the minute
    setSingleLEDColor(hour_led, CRGB::Green);
  }

  // Latch any new values into the LEDs
  void show() const {
    FastLED.show();
  }

private:
  CRGB _leds[kNumLEDs];
};
