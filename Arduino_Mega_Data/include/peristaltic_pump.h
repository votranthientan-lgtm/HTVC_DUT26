#ifndef PERISTALTIC_PUMP_H
#define PERISTALTIC_PUMP_H

#include <Arduino.h>

class PeristalticPump {
private:
    int _in1, _in2, _in3, _in4;
    bool _stateAB; // Quản lý cặp IN1-IN2
    bool _stateCD; // Quản lý cặp IN3-IN4

public:
    PeristalticPump(int in1, int in2, int in3, int in4);
    void begin();
    void control(float ph); 
    void turnOnAB();
    void turnOnCD();
    void stopAll();
    
    // Thêm 2 hàm này để bên mainmega gọi lấy dữ liệu gửi đi[cite: 3]
    bool getStateAB() { return _stateAB; }
    bool getStateCD() { return _stateCD; }
};

#endif