#include "ph_temp_sensor.h"

PhTempSensor::PhTempSensor(uint8_t phPin, uint8_t tempPin, float offset) 
    : _oneWire(tempPin), _sensors(&_oneWire) { 
    _phPin = phPin;
    _tempPin = tempPin;
    _offset = offset;
}

void PhTempSensor::begin() {
    pinMode(_phPin, INPUT);
    _sensors.begin(); 
}

void PhTempSensor::update() {
    // --- PHẦN 1: ĐO NHIỆT ĐỘ (DS18B20) ---
    _sensors.requestTemperatures(); 
    float t = _sensors.getTempCByIndex(0);
    if(t != DEVICE_DISCONNECTED_C) {
        _temperature = t;
    }

    // --- PHẦN 2: ĐO PH (GIỮ NGUYÊN CÔNG THỨC CỦA ÔNG) ---
    const int SAMPLES = 20; 
    int buf[SAMPLES];
    
    for(int i = 0; i < SAMPLES; i++) {
        buf[i] = analogRead(_phPin);
        delay(10);
    }

    // Thuật toán sắp xếp mảng của ông
    for(int i = 0; i < SAMPLES - 1; i++) {
        for(int j = i + 1; j < SAMPLES; j++) {
            if(buf[i] > buf[j]) {
                int temp = buf[i];
                buf[i] = buf[j];
                buf[j] = temp;
            }
        }
    }

    unsigned long avgValue = 0;
    for(int i = 2; i < SAMPLES - 2; i++) {
        avgValue += buf[i];
    }
    
    // Tính toán pH theo công thức ông đã viết
    float avgVoltage = (float)avgValue * 5.0 / 1024.0 / (SAMPLES - 4);
    _phValue = 7.0 + ( - avgVoltage) * 3.5 + _offset; 

    if (_phValue < 0) _phValue = 0;
    if (_phValue > 14) _phValue = 14;
}

float PhTempSensor::getPH() { return _phValue; }
float PhTempSensor::getTemperature() { return _temperature; }