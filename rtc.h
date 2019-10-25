// This header files contains all the RTC interfaces for the LED clock code

#include <stdio.h>
#include <Wire.h>
#include <RtcDS3231.h>

#define CLOCK_MODE_12H true
#define TIME_STRING_MAX_LENGTH 128
// Note: The RTC chip is connected over i2c to the default i2c pins on the
// atmega16 so that the TwoWire library just sets those up automatically.
// That's why there's no need to specify which pins it's on, like you might
// expect.

static RtcDS3231<TwoWire> Rtc(Wire);

// Setup the real time clock (RTC)
void rtc_setup() {
  Rtc.Begin();

  RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
  if (!Rtc.IsDateTimeValid()) {
    Serial.println("RTC lost confidence in the DateTime!");
    Rtc.SetDateTime(compiled);
  }

  if (!Rtc.GetIsRunning()) {
      Rtc.SetIsRunning(true);
  }
  Rtc.Enable32kHzPin(false);
  Rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
}


// Get the time
// This is just a wrapper around the RtcDS3231 library for API consistency
RtcDateTime rtc_getTime() {
  return Rtc.GetDateTime();
}

// Get the current time in a nice human-readable string format suitable for
// printing to the serial console, etc.
static char timeString[TIME_STRING_MAX_LENGTH];
char* rtc_getTimeString() {
  RtcDateTime now = rtc_getTime();
  sprintf(timeString,
          "%d:%02d:%02d",
          now.Hour() % 12, now.Minute(), now.Second());
  return timeString; 
}
