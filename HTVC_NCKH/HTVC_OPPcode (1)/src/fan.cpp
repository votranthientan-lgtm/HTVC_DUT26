#include "fan.h"

FanControl::FanControl(uint8_t pin) {
    _pin = pin; // Gán chân
    _state = false;
    _startTime = 0;
    _duration = 0;
}

void FanControl::begin() {
    digitalWrite(_pin, HIGH); // Xuất mức HIGH TRƯỚC để rơ-le luôn ngắt
    pinMode(_pin, OUTPUT);    // Sau đó mới thiết lập chân là OUTPUT
    _state = false;           // Đảm bảo biến trạng thái khớp với thực tế[cite: 6]
}

void FanControl::startFan(int minutes) {
    digitalWrite(_pin, LOW); // Bật quạt (Active Low)
    _state = true;
    _startTime = millis();
    _duration = (unsigned long)minutes * 60 * 1000;
}

void FanControl::turnOff() {
    digitalWrite(_pin, HIGH); // Tắt quạt
    _state = false;
    _duration = 0;
}

void FanControl::update() {
    if (_state && (millis() - _startTime >= _duration)) {
        turnOff();
    }
}

bool FanControl::getState() {
    return _state;
}