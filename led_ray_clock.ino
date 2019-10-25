#include <FastLED.h>
#define NUM_LEDS 6
#define DATA_PIN 0
CRGB leds[NUM_LEDS];

#include <Wire.h>
#include "DS3231.h"

#define BAUD_RATE 115200

#define CLOCK_MODE_12H true

// Which gpios are the two status LEDs connected to
constexpr int kBlinkyLEDPin = 24;

// Instantiate an object to control and read from the RTC over I2C
//DS3231 rtc;

void setup() {
  Serial.begin(BAUD_RATE);

  pinMode(kBlinkyLEDPin, OUTPUT);
  digitalWrite(kBlinkyLEDPin, LOW);

  LEDS.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  LEDS.setBrightness(64);

//  Wire.begin();
//
//  rtc.setClockMode(CLOCK_MODE_12H);
//  rtc.setSecond(0);
//  rtc.setHour(9);
//  rtc.setMinute(21);
}

//void blink(int ledPin) {
//  digitalWrite(ledPin, HIGH);
//  delay(250);
//  digitalWrite(ledPin, LOW);
//  delay(100);
//}
//
//void displayValue(int val, int ledPin) {
//  for (int i = 0; i < val; i++) {
//    blink(ledPin);
//  }
//}

void loop() {
  //bool h12, pm;
  //uint8_t hour = rtc.getHour(h12, pm);
  //displayValue(hour, kGreenLEDPin);
  //displayValue(rtc.getMinute(), kOrangeLEDPin);


  delay(500);
  Serial.println("test...");
  digitalWrite(kBlinkyLEDPin, LOW);
  fill_solid(leds, NUM_LEDS, CRGB(255, 128, 0));
  FastLED.show();

  delay(500);
  Serial.println("test...");
  digitalWrite(kBlinkyLEDPin, HIGH);
  fill_solid(leds, NUM_LEDS, CRGB(0, 128, 255));
  FastLED.show();
}
