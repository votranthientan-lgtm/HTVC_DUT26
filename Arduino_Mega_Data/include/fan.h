#ifndef FAN_H
#define FAN_H

#include <Arduino.h>

class FanControl {
private:
    uint8_t _pin; // Chỉ giữ lại 1 chân điều khiển
    bool _state;
    unsigned long _startTime;
    unsigned long _duration;

public:
    FanControl(uint8_t pin); // Constructor nhận 1 tham số
    void begin();
    void update(); 
    void startFan(int minutes); 
    void turnOff();
    bool getState();
};

#endif

