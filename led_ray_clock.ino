#include <TaskScheduler.h>
static Scheduler scheduler;

#include "LEDController.h"
static LEDController leds;

#include <stdio.h>
#include "Clock.h"
static Clock clock;

#include "Button.h"
// Button pin mappings on the PCB
constexpr uint8_t kButton1Pin = 28;
constexpr uint8_t kButton2Pin = 29;
constexpr uint8_t kButton3Pin = 30;
constexpr uint8_t kButton4Pin = 31;
// Functional button number mappings
enum {SET_TIME_BUTTON = 0,
      BRIGHTNESS_BUTTON = 1,
      UNUSED_BUTTON_ONE = 2,
      UNUSED_BUTTON_TWO = 3,
      NUM_BUTTONS = 4};
static Button buttons[NUM_BUTTONS];

// Brightness levels
constexpr uint8_t kBrightnessLevels[] = {10, 64, 128, 192, 255};
constexpr uint8_t kNumBrightnessLevels = sizeof(kBrightnessLevels);
static uint8_t brightnessLevel = kNumBrightnessLevels / 2;

#define BAUD_RATE 115200

// Which gpio is connected to the basic LED on the PCB (not RGB)
constexpr uint8_t kBlinkyLEDPin = 24;

// Task for updating the time
// This task runs every second and queries the current time from the RTC module
// and updates the static global value "now" for use in the other tasks.  Since
// this runs and updates that value the other taks never need to worry about
// when/how/where to get the time.  You can simply assume that "now" represents
// the current time.
static RtcDateTime now;
void updateTime() {
  now = clock.getTime();
  // Note: sprintf_P() allows you to store the format string in progmem
  // to save a little bit of memory
  constexpr uint8_t kMaxTimeStringLength = 9; // 00:00:00 is 8 characters + null
  char timeString[kMaxTimeStringLength];
  sprintf_P(timeString,
            PSTR("%d:%02d:%02d"),
            now.Hour() % 12, now.Minute(), now.Second());
  Serial.println(timeString);
}
Task taskUpdateTime(TASK_SECOND, TASK_FOREVER, &updateTime);

// Task to blink the power indicator
// This task simply blinks the power indicator LED. This is a standard/basic
// LED that's soldered directly onto the PCB and connected to a GPIO on the mcu.
// This is *not* with the RGB LEDs that make up the real clock itself.  This
// just blinks to let you know that everything is powered up and the scheduler
// is running/etc.
void updatePowerIndicator() {
  static bool powerIndicatorState = false;
  powerIndicatorState = !powerIndicatorState;
  digitalWrite(kBlinkyLEDPin, powerIndicatorState);
}
Task taskUpdatePowerIndicator(TASK_SECOND / 4, TASK_FOREVER, &updatePowerIndicator);

// Task to display something on the RGB LEDs.
// In this case, it just rainbow fades, but in the future this should actually
// render frames of the clock animation.
void updateMainLEDs() {
  // First clear out the LEDs and dimly light 12, 3, 6, and 9
  leds.fillSolid(CRGB::Black);

  // Put in a pole marker at noon, 3, 6, and 9
  leds.setPoleMarkers();

  // Display the time on top
  leds.displayTime(now);

  leds.show();
}
Task taskUpdateMainLEDs(TASK_SECOND, TASK_FOREVER, &updateMainLEDs);

// Callback to run continuously while the "set time" button is pressed down
// This adances the time faster than usual to let the user set the time.
static uint16_t timeSetButtonPressedDuration = 0;
void setTimeButtonWhileDownCallback() {
  Serial.print(F("Set Time Button Down!\tAdvancing time "));

  // Advance the amount of time that the button's been pressed down.
  // The conditional is just there to make sure that the integer doesn't wrap
  if (timeSetButtonPressedDuration <= 60000) {
    timeSetButtonPressedDuration++;
  }

  // Move the time forward a bit based on how long the button has been pressed
  // The longer it's been down, the faster time advances forward.
  uint8_t advanceStepSeconds = 0;
  if (timeSetButtonPressedDuration < 60) {
    advanceStepSeconds = 1;
  } else if (timeSetButtonPressedDuration < 4 * 60 + 60) {
    advanceStepSeconds = 7;
  } else {
    advanceStepSeconds = 70;
  }
  Serial.print(advanceStepSeconds);
  Serial.println(F(" seconds."));

  // Actually advance time and update it on the RTC
  now += advanceStepSeconds;
  clock.setTime(now);

  // Update the main LEDs to show the user the newly selected time
  updateMainLEDs();
}
// Callback to run when the brightness button is released
// This resets the timer that tracks how long the button has been down (since
// it's been released)
void setTimeButtonOnReleaseCallback() {
  Serial.println(F("Set Time Button Released!"));
  timeSetButtonPressedDuration = 0;
}

// Callback to run when the "brightness" button is pressed
// It cycles through the list of brightness levels, allowing the user to set
// their preferred level.
void brightnessButtonOnPressCallback() {
  Serial.println(F("Brightness Button Pressed!"));
  brightnessLevel = (brightnessLevel + 1) % kNumBrightnessLevels;
  Serial.print(F("New brightness level: "));
  Serial.println(brightnessLevel);
  leds.setBrightness(kBrightnessLevels[brightnessLevel]);
  leds.show();
}

// Periodic task that polls the buttons and runs their callbacks when appropriate
void checkButtonStateAndRunCallbacks() {
  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    bool success = buttons[i].checkStateAndRunCallbacks();
    if (!success) {
      Serial.print(F("Error: buttons["));
      Serial.print(i);
      Serial.println(F("] is not set up correctly"));
    }
  }
}
Task taskCheckButtonStates(TASK_MILLISECOND * 50, TASK_FOREVER, &checkButtonStateAndRunCallbacks);

// Set up the tasks defined above and get the scheduler ready to run.
void setupSchedulerTasks() {
  scheduler.addTask(taskUpdateTime);
  taskUpdateTime.enable();

  scheduler.addTask(taskUpdatePowerIndicator);
  taskUpdatePowerIndicator.enable();

  scheduler.addTask(taskUpdateMainLEDs);
  taskUpdateMainLEDs.enable();

  scheduler.addTask(taskCheckButtonStates);
  taskCheckButtonStates.enable();
}

void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println("Booting!");

  pinMode(kBlinkyLEDPin, OUTPUT);
  digitalWrite(kBlinkyLEDPin, LOW);

  clock.setup();
  leds.setup(kBrightnessLevels[brightnessLevel]);

  buttons[SET_TIME_BUTTON].setup(kButton1Pin, nullptr, setTimeButtonOnReleaseCallback, &setTimeButtonWhileDownCallback, nullptr);
  buttons[BRIGHTNESS_BUTTON].setup(kButton2Pin, &brightnessButtonOnPressCallback, nullptr, nullptr, nullptr);
  buttons[UNUSED_BUTTON_ONE].setup(kButton3Pin, nullptr, nullptr, nullptr, nullptr);
  buttons[UNUSED_BUTTON_TWO].setup(kButton4Pin, nullptr, nullptr, nullptr, nullptr);

  setupSchedulerTasks();
}

void loop() {
  scheduler.execute();
}
