#include <TaskScheduler.h>
static Scheduler scheduler;

#include "LEDController.h"
static LEDController leds;

#include "Clock.h"
static Clock clock;

#include "Button.h"
constexpr uint8_t kNumButtons = 2;
// Button pin mappings on the PCB
constexpr uint8_t kButton1Pin = 28;
constexpr uint8_t kButton2Pin = 29;
constexpr uint8_t kButton3Pin = 30;
constexpr uint8_t kButton4Pin = 31;
// Functional button number mappings
enum {SET_TIME_BUTTON = 0,
      BRIGHTNESS_BUTTON = 1};
static Button buttons[kNumButtons];

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
  Serial.println(clock.getTimeString(now));
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

// Task to poll the state of the buttons and run any button-press handlers
void setTimeButtonCallback() {
  Serial.println(F("Button 1 Pressed!"));
}
void brightnessButtonCallback() {
  Serial.println(F("Button 2 Pressed!"));
}
void checkButtonStateAndRunCallbacks() {
  for (uint8_t i = 0; i < kNumButtons; i++) {
    buttons[i].checkStateAndRunCallbacks();
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
  leds.setup();

  buttons[SET_TIME_BUTTON].setup(kButton1Pin, &setTimeButtonCallback);
  buttons[BRIGHTNESS_BUTTON].setup(kButton2Pin, &brightnessButtonCallback);

  setupSchedulerTasks();
}

void loop() {
  scheduler.execute();
}
