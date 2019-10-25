// Setup for the main LED strip
#include "LEDController.h"

// All the code for interfacing with the RTC (DS3231)
#include "rtc.h"

#define BAUD_RATE 115200

// Which gpio is connected to the basic LED on the PCB (not RGB)
constexpr int kBlinkyLEDPin = 24;

// This will be used to control the main RGB LED strip
LEDController leds;

void setup() {
  Serial.begin(BAUD_RATE);

  pinMode(kBlinkyLEDPin, OUTPUT);
  digitalWrite(kBlinkyLEDPin, LOW);

  rtc_setup();
}

void loop() {
  delay(500);
  Serial.println("test one...");
  Serial.println(rtc_getTimeString());
  leds.setSolid(CRGB(255, 128, 0));
  digitalWrite(kBlinkyLEDPin, LOW);

  delay(500);
  Serial.println("test two...");
  Serial.println(rtc_getTimeString());
  digitalWrite(kBlinkyLEDPin, HIGH);
  leds.setSolid(CRGB(0, 128, 255));
  FastLED.show();
}
