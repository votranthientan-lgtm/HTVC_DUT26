#include "led_control.h" // Chú ý: Viết thường cho khớp tên file

LedControl::LedControl(uint8_t pin) : _pin(pin), _state(false), _endTime(0) {}

void LedControl::begin() {
    pinMode(_pin, OUTPUT);
    digitalWrite(_pin, HIGH);
}

void LedControl::turnOn(int seconds) {
    _state = true;
    _endTime = millis() + (seconds * 1000ULL);
    digitalWrite(_pin, LOW);
}

void LedControl::update() {
    if (_state && millis() >= _endTime) {
        _state = false;
        digitalWrite(_pin, HIGH);
    }
}
void LedControl::turnOff() {
    _state = false;
    _endTime = 0;
    digitalWrite(_pin, HIGH);
}

bool LedControl::getState() {
    
    return _state;
}
