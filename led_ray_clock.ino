// All the code for interacing with the main RGB LED strip
#include "leds.h"

// All the code for interfacing with the RTC (DS3231)
#include "rtc.h"

#define BAUD_RATE 115200

// Which gpio is connected to the basic LED on the PCB (not RGB)
constexpr int kBlinkyLEDPin = 24;

void setup() {
  Serial.begin(BAUD_RATE);

  pinMode(kBlinkyLEDPin, OUTPUT);
  digitalWrite(kBlinkyLEDPin, LOW);

  rtc_setup();
  leds_setup();
}

void loop() {
  delay(500);
  Serial.println("test one...");
  Serial.println(rtc_getTimeString());
  leds_setSolid(CRGB(255, 128, 0));
  digitalWrite(kBlinkyLEDPin, LOW);

  delay(500);
  Serial.println("test two...");
  Serial.println(rtc_getTimeString());
  digitalWrite(kBlinkyLEDPin, HIGH);
  leds_setSolid(CRGB(0, 128, 255));
  FastLED.show();
}
