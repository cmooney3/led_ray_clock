#include <Wire.h>
#include "DS3231.h"

#define CLOCK_MODE_12H true

// Which gpios are the two status LEDs connected to
constexpr int kBlinkyLEDPin = 24;

// Instantiate an object to control and read from the RTC over I2C
//DS3231 rtc;

void setup() {
  pinMode(kBlinkyLEDPin, OUTPUT);
  digitalWrite(kBlinkyLEDPin, LOW);

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


  delay(2000);
  digitalWrite(kBlinkyLEDPin, LOW);
  delay(2000);
  digitalWrite(kBlinkyLEDPin, HIGH);
}
