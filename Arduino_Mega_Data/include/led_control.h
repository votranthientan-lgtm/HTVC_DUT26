#ifndef LED_CONTROL_H
#define LED_CONTROL_H
#include <Arduino.h>

class LedControl {
private:
    uint8_t _pin;
    bool _state;
    unsigned long _endTime;

public:
    void turnOff(); //[cite: 5]
    LedControl(uint8_t pin);
    void begin();
    void turnOn(int seconds);
    void update();
    bool getState();
};
#endif