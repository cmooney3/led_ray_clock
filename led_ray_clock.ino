#include <TaskScheduler.h>
Scheduler scheduler;

#include "LEDController.h"
LEDController leds;

#include "Clock.h"
Clock clock;

#define BAUD_RATE 115200

// Which gpio is connected to the basic LED on the PCB (not RGB)
constexpr int kBlinkyLEDPin = 24;

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

//  unsigned int second_led = now.Second() * kNumLEDs / 60;
//  leds[(kNumLEDs - second_led + kClockRotationalOffsetToNoon) % kNumLEDs] = CRGB::Red;
//
//  unsigned int minute_led = now.Minute() * kNumLEDs / 60;
//  leds[(kNumLEDs - minute_led + kClockRotationalOffsetToNoon) % kNumLEDs] = CRGB::Green;
//
//  unsigned int hour_led = (now.Hour() % 12) * kNumLEDs / 12;
//  hour_led += (kNumLEDs * now.Minute()) / (12 * 60); // Advance within the hour based on the minute
//  leds[(kNumLEDs - hour_led + kClockRotationalOffsetToNoon) % kNumLEDs] = CRGB::Blue;

  leds.show();
}
Task taskUpdateMainLEDs(TASK_SECOND, TASK_FOREVER, &updateMainLEDs);

// Set up the tasks defined above and get the scheduler ready to run.
void setupSchedulerTasks() {
  scheduler.addTask(taskUpdateTime);
  taskUpdateTime.enable();

  scheduler.addTask(taskUpdatePowerIndicator);
  taskUpdatePowerIndicator.enable();

  scheduler.addTask(taskUpdateMainLEDs);
  taskUpdateMainLEDs.enable();
}

void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println("Booting!");

  pinMode(kBlinkyLEDPin, OUTPUT);
  digitalWrite(kBlinkyLEDPin, LOW);

  clock.setup();
  leds.setup();

  setupSchedulerTasks();
}

void loop() {
  scheduler.execute();
}
