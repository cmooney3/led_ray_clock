// Constants to abstract away which boolean value is pressed/released
constexpr bool kButtonPressed = false;
constexpr bool kButtonReleased = true;

// Which gpio's are connected to the buttons and some memory to store their values
constexpr uint8_t kNoPinSetupYet = 0xFF;

typedef void (*ButtonHandlerCallback)();

class Button {
public:
  Button() : _pin(kNoPinSetupYet), _state(kButtonReleased), _onPress(nullptr) {}

  void setup(uint8_t pin, ButtonHandlerCallback onPress) {
    _pin = pin;
    _onPress = onPress;

    pinMode(_pin, INPUT_PULLUP);
    _state = getState();
  }

  void checkStateAndRunCallbacks() {
    // If this button hasn't been setup and/or doesn't have a callback then
    // there's no reason to do anything here.
    if (_pin == kNoPinSetupYet || _onPress == nullptr) {
      return;
    }

    // Get the current state of the button, and run a callback if the state changed
    bool newState = getState();
    if (newState != _state) {
      if (newState == kButtonPressed) {
        // Button Pressed
        _onPress();
      } else {
        // Button Released
        // TODO: Possibly add an onRelease callback to call here
      }
    }

    // Update the state with the newly collected state
    _state = newState;
  }

private:
  bool getState() const {
    return digitalRead(_pin);
  }

  uint8_t _pin;
  bool _state;
  ButtonHandlerCallback _onPress;
};
