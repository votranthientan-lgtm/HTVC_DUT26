#ifndef PUMP_CONTROL_H
#define PUMP_CONTROL_H
#include <Arduino.h>

class PumpControl {
private:
    uint8_t _pinA, _pinB, _pinC;
    bool _isIrrigating;
    unsigned long _startTime;
    unsigned long _duration;
public:
    // Khởi tạo với 3 chân cho 3 bơm
    PumpControl(uint8_t a, uint8_t b, uint8_t c);
    void begin();
    void turnOnManual();
    void stopAll();
    void startIrrigation(int duration); // Bật cả 3 bơm cùng lúc
    void update();
    bool getState();
};

#endif