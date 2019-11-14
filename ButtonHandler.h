// Constants to abstract away which boolean value is pressed/released
constexpr bool kButtonPressed = false;
constexpr bool kButtonReleased = false;

// Which gpio's are connected to the buttons and some memory to store their values
constexpr uint8_t kNumButtons = 4;
constexpr uint8_t kButton1Pin = 28;
constexpr uint8_t kButton2Pin = 29;
constexpr uint8_t kButton3Pin = 30;
constexpr uint8_t kButton4Pin = 31;

class ButtonHandler {
public:
  void setup() {
    _buttons[0].setup(kButton1Pin);
//    _buttons[1].setup(kButton2Pin);
//    _buttons[2].setup(kButton3Pin);
//    _buttons[3].setup(kButton4Pin);
  }

  void checkStates() {
    _buttons[0].update();
  }

private:
  class Button {
  public:
    Button() : _pin(0), _state(kButtonReleased) {}

    void setup(uint8_t pin) {
      _pin = pin;
      pinMode(_pin, INPUT_PULLUP);
    }

    void update() {
      bool newState = digitalRead(_pin);
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
    uint8_t _pin;
    bool _state;
  };

  Button _buttons[kNumButtons];
};
