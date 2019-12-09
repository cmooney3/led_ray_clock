// This header files contains all the RTC interfaces for the LED clock code

#include <Wire.h>
#include <RtcDS3231.h>

// Note: The RTC chip is connected over i2c to the default i2c pins on the
// atmega16 so that the TwoWire library just sets those up automatically.
// That's why there's no need to specify which pins it's on, like you might
// expect.


// I dont know what this is...  Something in the clock library I'm using is
// clobbering memory after it.  This is a number of bytes I have to allocate
// after the Clock to provide a buffer for this overrun. It's super hacky,
// but it seems to work and I can't figure out what this library is doing
// wrong.
constexpr uint8_t kUnexplainedMemoryBufferSize = 50;

class Clock {
public:
    Clock() : _rtc(Wire) {}

    void setup() {
        _rtc.Begin();
    
        RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
        if (!_rtc.IsDateTimeValid()) {
            _rtc.SetDateTime(compiled);
        }
    
        if (!_rtc.GetIsRunning()) {
            _rtc.SetIsRunning(true);
        }
        _rtc.Enable32kHzPin(false);
        _rtc.SetSquareWavePin(DS3231SquareWavePin_ModeNone);
    }

    RtcDateTime getTime() {
        return _rtc.GetDateTime();
    }

    void setTime(const RtcDateTime& time) {
        _rtc.SetDateTime(time);
    }

private:
    RtcDS3231<TwoWire> _rtc;
    uint8_t buffer[kUnexplainedMemoryBufferSize];
};
