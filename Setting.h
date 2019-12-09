#pragma once

#include <EEPROM.h>

typedef uint16_t EEPROMAddress;

// A little class to handle saving/retreiving single-byte settings from EEPROM
// This makes it simple and easy to save things like brightness levels and
// color settings/etc to non volatile memory so when you reboot the system they
// can be restored automatically
class Setting {
public:
    Setting(EEPROMAddress addr, uint8_t numValues) {
        _addr = addr;
        readValue();

        // If the number is out of range, set the setting to 0
        if (_value >= numValues) {
            setValue(0);
        }
    }

    void setValue(uint8_t newValue) {
        EEPROM.update(_addr, newValue);
        _value = newValue;
    }

    uint8_t getValue() {
        return _value;
    }

    void readValue() {
        _value = EEPROM.read(_addr);
    }

private:
    EEPROMAddress _addr;
    uint8_t _value;
};
