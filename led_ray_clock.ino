#include <TaskScheduler.h>
Scheduler scheduler;

#include "LEDController.h"
LEDController *leds;

#include "Clock.h"
Clock* clock;

#define BAUD_RATE 115200

// Which gpio is connected to the basic LED on the PCB (not RGB)
constexpr int kBlinkyLEDPin = 24;

// Task for updating the time
void updateTime() {
  Serial.println(clock->getTimeString());
}
Task taskUpdateTime(TASK_SECOND, TASK_FOREVER, &updateTime);

// Task to blink the power indicator
void updatePowerIndicator() {
  static bool powerIndicatorState = false;
  powerIndicatorState = !powerIndicatorState;
  digitalWrite(kBlinkyLEDPin, powerIndicatorState);
}
Task taskUpdatePowerIndicator(TASK_SECOND / 4, TASK_FOREVER, &updatePowerIndicator);

// Task to display something on the RGB LEDs.  In this case, it just rainbow fades
void updateMainLEDs() {
  static uint8_t hue = 0;
  leds->setSolid(CHSV(hue++, 200, 128));
}
Task taskUpdateMainLEDs(TASK_SECOND / 32, TASK_FOREVER, &updateMainLEDs);

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

  clock = new Clock();
  leds = new LEDController();

  setupSchedulerTasks();
}

void loop() {
  scheduler.execute();
}
