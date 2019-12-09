#include <TaskScheduler.h>
static Scheduler scheduler;

#include "LEDController.h"
static LEDController leds;

#include <stdio.h>
#include "Clock.h"
static Clock clock;

// The various color sets that can be selected for the clock hands
typedef struct color_set {
    CRGB second_hand;
    CRGB minute_hand;
    CRGB hour_hand;
} ColorSet;
const ColorSet kColorSets[] = {
    {CRGB::Purple, CRGB::Green, CRGB::Blue},
    {CRGB::Purple, CRGB::Green, CRGB::Cyan},
    {CRGB::Green, CRGB::Purple, CRGB::Cyan},
//    {CRGB::Orange, CRGB::Red, CRGB::Yellow},
//    {CRGB::Yellow, CRGB::Orange, CRGB::Red},
};
const uint8_t kNumColorSets = sizeof(kColorSets) / sizeof(ColorSet);

#include "Button.h"
// Button pin mappings on the PCB
constexpr uint8_t kButton1Pin = 28;
constexpr uint8_t kButton2Pin = 29;
constexpr uint8_t kButton3Pin = 30;
constexpr uint8_t kButton4Pin = 31;
// Functional button number mappings
enum { SET_TIME_BUTTON = 0,
       BRIGHTNESS_BUTTON = 1,
       PATTERN_BUTTON = 2,
       COLOR_BUTTON = 3,
       NUM_BUTTONS = 4 };
static Button buttons[NUM_BUTTONS];

// Brightness levels
constexpr uint8_t kBrightnessLevels[] = {10, 64, 128, 192, 255};
constexpr uint8_t kNumBrightnessLevels = sizeof(kBrightnessLevels);

// Setting up constants to indicate the various kind of clock patterns
constexpr uint8_t kShowPoles = 0b00000001;
constexpr uint8_t kShowSecondHand = 0b00000010;
enum { NO_POLES_AND_NO_SECOND_HAND = 0,
       POLES_AND_NO_SECOND_HAND = kShowPoles,
       NO_POLES_AND_SECOND_HAND = kShowSecondHand,
       POLES_AND_SECOND_HAND = kShowPoles | kShowSecondHand,
       NUM_PATTERNS = 4 };


#include "Setting.h"
constexpr EEPROMAddress kBrightnessLevelSettingAddress = 0x0000;
constexpr EEPROMAddress kPatternSettingAddress = 0x0002;
constexpr EEPROMAddress kColorSettingAddress = 0x0004;
static Setting brightnessLevelSetting(kBrightnessLevelSettingAddress, kNumBrightnessLevels);
static Setting patternSetting(kPatternSettingAddress, NUM_PATTERNS);
static Setting colorSetting(kColorSettingAddress, kNumColorSets);

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
    // First clear out the LEDs
    leds.fillSolid(CRGB::Black);
  
    // Put in a pole marker at noon, 3, 6, and 9 (if the current pattern setting includes them)
    if (patternSetting.getValue() & kShowPoles) {
        leds.setPoleMarkers();
    }
  
    // Display the time on the clock face one hand at a time
    if (patternSetting.getValue() & kShowSecondHand) {
        // Only display the second hand if the currently selected pattern includes it
        leds.displaySecondHand(now, kColorSets[colorSetting.getValue()].second_hand);
    }
    // Always show the minute and hour hands -- you really need those
    leds.displayMinuteHand(now, kColorSets[colorSetting.getValue()].minute_hand);
    leds.displayHourHand(now, kColorSets[colorSetting.getValue()].hour_hand);
  
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
      brightnessLevelSetting.setValue((brightnessLevelSetting.getValue() + 1) % kNumBrightnessLevels);
      Serial.print(F("\tNew brightness level: "));
      Serial.println(brightnessLevelSetting.getValue());
      leds.setBrightness(kBrightnessLevels[brightnessLevelSetting.getValue()]);
      leds.show();
}

// Callback to run when the "pattern" button is pressed
// It cycles through a few different settings to control the "pattern" that the clock uses.
// Specifically it turns the pole markers and the second hand on/off
void patternButtonOnPressCallback() {
    Serial.println(F("Pattern Button Pressed!"));
    patternSetting.setValue((patternSetting.getValue() + 1) % NUM_PATTERNS);
    Serial.print(F("\tNew pattern selected: "));
    Serial.println(patternSetting.getValue());
    Serial.print(F("\tPole markers:\t"));
    Serial.println(static_cast<bool>(patternSetting.getValue() & kShowPoles) ? F("on") : F("off"));
    Serial.print(F("\tSecond hand:\t"));
    Serial.println(static_cast<bool>(patternSetting.getValue() & kShowSecondHand) ? F("on") : F("off"));

    // Update the display so the new setting takes effect immediately
    updateMainLEDs();
}

// Callback to run when the "Color" button is pressed
// It cycles through a few different color sets to be used for the clock hands
void colorButtonOnPressCallback() {
    //Serial.println(F("Color Button Pressed!"));
    colorSetting.setValue((colorSetting.getValue() + 1) % kNumColorSets);
    //Serial.print(F("\tNew color set selected: "));
    //Serial.println(colorSetting.getValue());

    // Update the display so the new colors take effect immediately
    updateMainLEDs();
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
    Serial.println(F("Booting!"));

    pinMode(kBlinkyLEDPin, OUTPUT);
    digitalWrite(kBlinkyLEDPin, LOW);

    clock.setup();
    leds.setup(kBrightnessLevels[brightnessLevelSetting.getValue()]);

    buttons[SET_TIME_BUTTON].setup(kButton1Pin, nullptr, setTimeButtonOnReleaseCallback, &setTimeButtonWhileDownCallback, nullptr);
    buttons[BRIGHTNESS_BUTTON].setup(kButton2Pin, &brightnessButtonOnPressCallback, nullptr, nullptr, nullptr);
    buttons[PATTERN_BUTTON].setup(kButton3Pin, &patternButtonOnPressCallback, nullptr, nullptr, nullptr);
    buttons[COLOR_BUTTON].setup(kButton4Pin, &colorButtonOnPressCallback, nullptr, nullptr, nullptr);

    brightnessLevelSetting.readValue();
    patternSetting.readValue();

    setupSchedulerTasks();
}

void loop() {
    scheduler.execute();
}
