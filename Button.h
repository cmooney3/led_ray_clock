// Constants to abstract away which boolean value is pressed/released
constexpr bool kButtonPressed = false;
constexpr bool kButtonReleased = true;

// Which gpio's are connected to the buttons and some memory to store their values
constexpr uint8_t kNoPinSetupYet = 0xFF;

typedef void (*ButtonHandlerCallback)();

class Button {
public:
    Button() : _pin(kNoPinSetupYet), _state(kButtonReleased), _onPress(nullptr) {}

    void setup(uint8_t pin,
               ButtonHandlerCallback onPress, ButtonHandlerCallback onRelease,
               ButtonHandlerCallback whileDown, ButtonHandlerCallback whileUp) {
        _pin = pin;
        _onPress = onPress;
        _onRelease = onRelease;
        _whileDown = whileDown;
        _whileUp = whileUp;

        pinMode(_pin, INPUT_PULLUP);
        _state = getState();
    }

    bool checkStateAndRunCallbacks() {
        // If this button hasn't been setup and/or doesn't have a callback then
        // there's no reason to do anything here.  Return "false" to indicate a problem
        if (_pin == kNoPinSetupYet) {
            return false;
        }

        bool newState = getState();

        // First check if there are any edge-triggered callbacks to run
        if (newState != _state) {
            if (newState == kButtonPressed) {
                // Button Pressed
                if (_onPress != nullptr) {
                    _onPress();
                }
            } else {
                // Button Released
                if (_onRelease != nullptr) {
                    _onRelease();
                }
            }
        }

        // Update the state with the newly collected state
        _state = newState;

        // Finally run the state-based callbacks
        if (_state == kButtonPressed) {
            if (_whileDown != nullptr) {
              _whileDown();
            }
        } else {
            if (_whileUp != nullptr) {
                _whileUp();
            }
        }

        return true;
    }

private:
    bool getState() const {
        return digitalRead(_pin);
    }

    uint8_t _pin;
    bool _state;
    ButtonHandlerCallback _onPress, _onRelease, _whileDown, _whileUp;
};
