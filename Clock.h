// This header files contains all the RTC interfaces for the LED clock code

#include <stdio.h>
#include <Wire.h>
#include <RtcDS3231.h>

#define TIME_STRING_MAX_LENGTH 128
// Note: The RTC chip is connected over i2c to the default i2c pins on the
// atmega16 so that the TwoWire library just sets those up automatically.
// That's why there's no need to specify which pins it's on, like you might
// expect.

class Clock {
public:
  Clock() {
    _rtc = new RtcDS3231<TwoWire>(Wire);
    _rtc->Begin();
  
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    if (!_rtc->IsDateTimeValid()) {
      _rtc->SetDateTime(compiled);
    }
  
    if (!_rtc->GetIsRunning()) {
        _rtc->SetIsRunning(true);
    }
    _rtc->Enable32kHzPin(false);
    _rtc->SetSquareWavePin(DS3231SquareWavePin_ModeNone);
  }


  RtcDateTime getTime() {
    return _rtc->GetDateTime();
  }

  char* getTimeString() {
    RtcDateTime now = getTime();
    sprintf(_timeString,
            "%d:%02d:%02d",
            now.Hour() % 12, now.Minute(), now.Second());
    return _timeString; 
  }

private:
  RtcDS3231<TwoWire>* _rtc;
  char _timeString[TIME_STRING_MAX_LENGTH];
};
