#include "LEDController.h"
LEDController *leds;

#include "Clock.h"
Clock* clock;

#define BAUD_RATE 115200

// Which gpio is connected to the basic LED on the PCB (not RGB)
constexpr int kBlinkyLEDPin = 24;

void setup() {
  Serial.begin(BAUD_RATE);
  Serial.println("Booting!");

  pinMode(kBlinkyLEDPin, OUTPUT);
  digitalWrite(kBlinkyLEDPin, LOW);

  clock = new Clock();
  leds = new LEDController();
}

void loop() {
  delay(500);
  Serial.println("test one...");
  Serial.println(clock->getTimeString());
  leds->setSolid(CRGB(255, 128, 0));
  digitalWrite(kBlinkyLEDPin, LOW);

  delay(500);
  Serial.println("test two...");
  Serial.println(clock->getTimeString());
  leds->setSolid(CRGB(0, 128, 255));
  digitalWrite(kBlinkyLEDPin, HIGH);
}
