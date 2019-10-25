#include <FastLED.h>
#define NUM_LEDS 6
#define DATA_PIN 0
CRGB leds[NUM_LEDS];

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

  LEDS.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  LEDS.setBrightness(64);
}

void loop() {
  delay(500);
  Serial.println("test one...");
  Serial.println(rtc_getTimeString());
  digitalWrite(kBlinkyLEDPin, LOW);
  fill_solid(leds, NUM_LEDS, CRGB(255, 128, 0));
  FastLED.show();

  delay(500);
  Serial.println("test two...");
  Serial.println(rtc_getTimeString());
  digitalWrite(kBlinkyLEDPin, HIGH);
  fill_solid(leds, NUM_LEDS, CRGB(0, 128, 255));
  FastLED.show();
}
