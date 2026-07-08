#include "pump_control.h"

PumpControl::PumpControl(uint8_t a, uint8_t b, uint8_t c) 
    : _pinA(a), _pinB(b), _pinC(c), _isIrrigating(false), _startTime(0), _duration(0) {}

void PumpControl::begin() {
    pinMode(_pinA, OUTPUT);
    pinMode(_pinB, OUTPUT);
    pinMode(_pinC, OUTPUT);
    stopAll(); // Mới bật máy lên thì tắt hết bơm
}

void PumpControl::stopAll() {
    // Với Relay Active Low: HIGH là ngắt điện
    digitalWrite(_pinA, HIGH);
    digitalWrite(_pinB, HIGH);
    digitalWrite(_pinC, HIGH);
    _isIrrigating = false;
    _startTime = 0;
    _duration = 0;
}

void PumpControl::turnOnManual() {
    _isIrrigating = true;
    _startTime = 0;
    _duration = 0;
    digitalWrite(_pinA, LOW);
    digitalWrite(_pinB, LOW);
    digitalWrite(_pinC, LOW);
}

void PumpControl::startIrrigation(int duration) {
    if (duration <= 0) {
        stopAll();
        return;
    }

    _isIrrigating = true;
    _startTime = millis();
    _duration = static_cast<unsigned long>(duration);
    digitalWrite(_pinA, LOW);
    digitalWrite(_pinB, LOW);
    digitalWrite(_pinC, LOW);
}

void PumpControl::update() {
    if (_isIrrigating && _duration > 0 && (millis() - _startTime >= _duration)) {
        stopAll();
    }
}

bool PumpControl::getState() {
    return _isIrrigating;
}