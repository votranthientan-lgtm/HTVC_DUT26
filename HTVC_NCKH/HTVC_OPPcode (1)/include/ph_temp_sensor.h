#ifndef PH_TEMP_SENSOR_H
#define PH_TEMP_SENSOR_H

#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

class PhTempSensor {
private:
    uint8_t _phPin;
    uint8_t _tempPin;
    float _offset;
    float _phValue;
    float _temperature;

    OneWire _oneWire;
    DallasTemperature _sensors;

public:
    // Khởi tạo khớp với: myPhSensor(A0, 13, 0.70)
    PhTempSensor(uint8_t phPin, uint8_t tempPin, float offset);
    void begin();
    void update(); 
    float getPH();
    float getTemperature();
};

#endif