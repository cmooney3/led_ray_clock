// Constants to abstract away which boolean value is pressed/released
constexpr bool kButtonPressed = false;
constexpr bool kButtonReleased = true;

// Which gpio's are connected to the buttons and some memory to store their values
constexpr uint8_t kNumButtons = 4;
constexpr uint8_t kNoPinSetupYet = 0xFF;
constexpr uint8_t kButton1Pin = 28;
constexpr uint8_t kButton2Pin = 29;
constexpr uint8_t kButton3Pin = 30;
constexpr uint8_t kButton4Pin = 31;

class ButtonHandler {
public:
  void setup() {
    _buttons[0].setup(kButton1Pin);
    _buttons[1].setup(kButton2Pin);
    _buttons[2].setup(kButton3Pin);
    _buttons[3].setup(kButton4Pin);
  }

  void checkStates() {
    for (uint8_t i = 0; i < kNumButtons; i++) {
      _buttons[i].checkState();
    }
  }

private:
  class Button {
  public:
    Button() : _pin(kNoPinSetupYet), _state(kButtonReleased) {}

    void setup(uint8_t pin) {
      _pin = pin;
      pinMode(_pin, INPUT_PULLUP);
      _state = getState();
    }

    void checkState() {
      if (_pin == kNoPinSetupYet) {
        return;
      }

      bool newState = getState();
      if (newState != _state) {
        if (newState == kButtonPressed) {
          Serial.println(F("Button pressed"));
        } else {
          Serial.println(F("Button released"));
        }
      }
      _state = newState;
    }

  private:
    bool getState() const {
      return digitalRead(_pin);
    }

    uint8_t _pin;
    bool _state;
  };

  Button _buttons[kNumButtons];
};
