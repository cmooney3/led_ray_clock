#define FASTLED_INTERNAL  // Supress pragma warnings about not using HW SPI for FastLED control
#include <FastLED.h>

#include <TaskScheduler.h>
Scheduler scheduler;

#include <Wire.h>
#include <RtcDS3231.h>
RtcDS3231<TwoWire> Rtc(Wire);

// Pin assignments
static constexpr uint8_t kButtonPin = 0;
static constexpr uint8_t kLEDPin = 13;

// LED details
constexpr uint16_t kBrightness = 255;
constexpr uint16_t kNumLEDs = 36; // How Many LEDs are connected
#define LED_TYPE WS2812B
CRGB leds[kNumLEDs];

// How far Noon is from the start of the LED strip (number of LEDs)
static constexpr uint8_t kClockRotationalOffsetToNoon = 8;

void setupFastLED() {
  // Configure FastLED for the main RGB LED strip that this unit controls
  FastLED.addLeds<LED_TYPE, kLEDPin, GRB>(leds, kNumLEDs);

  // Set all the LEDs to black (aka off) so that the LEDs don't flash random colors on boot
  fill_solid(leds, kNumLEDs, CRGB::Black);
  FastLED.setBrightness(kBrightness);
  FastLED.show();
}

void setupRTC() {
  // Setup the real time clock (RTC)
  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  if (!Rtc.IsDateTimeValid()) {
    Serial.println("RTC lost confidence in the DateTime!");
    Rtc.SetDateTime(compiled);
  }
}

RtcDateTime now;
void updateTime() {
  Serial.println("Querying RTC for time.");
  now = Rtc.GetDateTime();
  Serial.print(now.Hour() % 12);
  Serial.print(":");
  Serial.print(now.Minute());
  Serial.print(":");
  Serial.println(now.Second());
}
Task taskUpdateTime(TASK_SECOND, TASK_FOREVER, &updateTime);

void updateDisplay() {
  // Update the display with the current time
  Serial.println("Displaying time.");

  // First clear out the LEDs and dimly light 12, 3, 6, and 9
  fill_solid(leds, kNumLEDs, CRGB::Black);
  CRGB kMarkerColor = CHSV(0, 0, 16);
  leds[(0 + kClockRotationalOffsetToNoon) % kNumLEDs] = kMarkerColor;
  leds[(kNumLEDs / 4 + kClockRotationalOffsetToNoon) % kNumLEDs] = kMarkerColor;
  leds[(kNumLEDs / 2 + kClockRotationalOffsetToNoon) % kNumLEDs] = kMarkerColor;
  leds[(3 * kNumLEDs / 4 + kClockRotationalOffsetToNoon) % kNumLEDs] = kMarkerColor;

  unsigned int second_led = now.Second() * kNumLEDs / 60;
  leds[(kNumLEDs - second_led + kClockRotationalOffsetToNoon) % kNumLEDs] = CRGB::Red;

  unsigned int minute_led = now.Minute() * kNumLEDs / 60;
  leds[(kNumLEDs - minute_led + kClockRotationalOffsetToNoon) % kNumLEDs] = CRGB::Green;

  unsigned int hour_led = (now.Hour() % 12) * kNumLEDs / 12;
  hour_led += (kNumLEDs * now.Minute()) / (12 * 60); // Advance within the hour based on the minute
  leds[(kNumLEDs - hour_led + kClockRotationalOffsetToNoon) % kNumLEDs] = CRGB::Blue;

  FastLED.show();
}
Task taskUpdateDisplay(TASK_SECOND, TASK_FOREVER, &updateDisplay);

void fastForwardTime() {
//  static int pressed_duration = 0;
//
//  // If the button is pressed down advance time by increasingly larger chunks
//  if (!digitalRead(kButtonPin)) {
//    pressed_duration++;
//    if (pressed_duration < 60) {
//      now = now + TimeSpan(1);
//    } else if (pressed_duration < 7 * 60 + 60) {
//      now = now + TimeSpan(7);
//    } else {
//      now = now + TimeSpan(70);
//    }
//    updateDisplay();
//  } else {
//    pressed_duration = 0;
//  }
}
Task taskFastForwardTime(TASK_SECOND / 100, TASK_FOREVER, &fastForwardTime);

void setup() {
  Serial.begin(115200);
  Serial.println("Booting...");

  // Set up the brightness button.
  pinMode(kButtonPin, INPUT_PULLUP);

  // Set up FastLED to control the actual LEDs.  This makes them enabled but
  // turns them all off, so it won't start as a big flash of random colors.
  setupFastLED();

  // Set up the real time clock.  This mostly means configuring i2c and programming
  // in a time if the RTC has lost power at some point.
  setupRTC();

  // Set up a task to update the time
  scheduler.addTask(taskUpdateTime);
  taskUpdateTime.enable();

  // Set up a task to update the LED time periodically.
  scheduler.addTask(taskUpdateDisplay);
  taskUpdateDisplay.enable();

  // Advance time when the button is pressed down.
  scheduler.addTask(taskFastForwardTime);
  taskFastForwardTime.enable();

  Serial.println("Set up complete!  Entering main program loop now.");
  Serial.println();
}

void loop() {
  scheduler.execute();
}
