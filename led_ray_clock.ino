#define FASTLED_INTERNAL  // Supress pragma warnings about not using HW SPI for FastLED control
#include <FastLED.h>

#include <TaskScheduler.h>
Scheduler scheduler;

#include <Wire.h>
#include <RTClib.h>
RTC_DS3231 rtc;
static constexpr uint8_t kRtcSdaPin = 0;
static constexpr uint8_t kRtcSclPin = 2;

// Pin assignments
static constexpr uint8_t kButtonPin = 0;
static constexpr uint8_t kLEDPin = 5;

// LED details
constexpr uint16_t kNumLEDs = 36; // How Many LEDs are connected
#define LED_TYPE WS2812B
CRGB leds[kNumLEDs];

void setupFastLED() {
  // Configure FastLED for the main RGB LED strip that this unit controls
  FastLED.addLeds<LED_TYPE, kLEDPin, GRB>(leds, kNumLEDs);

  // Set all the LEDs to black (aka off) so that the LEDs don't flash random colors on boot
  fill_solid(leds, kNumLEDs, CRGB::Black);
  FastLED.setBrightness(128);
  FastLED.show();
}

void setupRTC() {
  // Setup the real time clock (RTC)

  // The RTC works over I2C so we set up the Arduino Wire library on the I2c pins we're using.
  Wire.begin(kRtcSdaPin, kRtcSclPin);  // Use this instead of rtc.begin() so I can set the pins

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, ReSetting the time to 12:00!");
    rtc.adjust(DateTime(2019, 1, 1, 0, 0, 0));
  }
}

DateTime now(2019, 1, 1, 0, 0, 0);
void updateTime() {
  now = now + TimeSpan(1); // Advance 1 second
}
Task taskUpdateTime(TASK_SECOND, TASK_FOREVER, &updateTime);

void updateDisplay() {
  // Update the display with the current time
  fill_solid(leds, kNumLEDs, CRGB::Black);

  unsigned int second_led = now.second() * kNumLEDs / 60;
  leds[kNumLEDs - second_led - 1] = CRGB::Red;

  unsigned int minute_led = now.minute() * kNumLEDs / 60;
  leds[kNumLEDs - minute_led - 1] = CRGB::Green;

  unsigned int hour_led = now.hour() * kNumLEDs / 12;
  leds[kNumLEDs - hour_led - 1] = CRGB::Blue;

  FastLED.show();
}
Task taskUpdateDisplay(TASK_SECOND, TASK_FOREVER, &updateDisplay);

void fastForwardTime() {
  static int pressed_duration = 0;

  // If the button is pressed down advance time by increasingly larger chunks
  if (!digitalRead(kButtonPin)) {
    pressed_duration++;
    if (pressed_duration < 60) {
      now = now + TimeSpan(1);
    } else if (pressed_duration < 7 * 60 + 60) {
      now = now + TimeSpan(7);
    } else {
      now = now + TimeSpan(70);
    }
    updateDisplay();
  } else {
    pressed_duration = 0;
  }
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
