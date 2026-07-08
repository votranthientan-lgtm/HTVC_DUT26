#include "peristaltic_pump.h"

PeristalticPump::PeristalticPump(int in1, int in2, int in3, int in4) {
    _in1 = in1; _in2 = in2; _in3 = in3; _in4 = in4;
}

void PeristalticPump::begin() {
    pinMode(_in1, OUTPUT); pinMode(_in2, OUTPUT);
    pinMode(_in3, OUTPUT); pinMode(_in4, OUTPUT);
    stopAll();
}

void PeristalticPump::stopAll() {
    digitalWrite(_in1, LOW); digitalWrite(_in2, LOW);
    digitalWrite(_in3, LOW); digitalWrite(_in4, LOW);
    _stateAB = false; _stateCD = false;
}

void PeristalticPump::turnOnAB() {
    digitalWrite(_in1, HIGH); digitalWrite(_in2, LOW);
    digitalWrite(_in3, LOW);  digitalWrite(_in4, LOW);
    _stateAB = true; _stateCD = false;
}

void PeristalticPump::turnOnCD() {
    digitalWrite(_in1, LOW);  digitalWrite(_in2, LOW);
    digitalWrite(_in3, HIGH); digitalWrite(_in4, LOW);
    _stateAB = false; _stateCD = true;
}

void PeristalticPump::control(float ph) {
    if (ph < 6.8) {
        turnOnAB();
    } 
    else if (ph > 7.2) {
        turnOnCD();
    } 
    else {
        stopAll();
    }
}