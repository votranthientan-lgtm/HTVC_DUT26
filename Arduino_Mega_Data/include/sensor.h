#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>
#include <DHT.h>

#define SCOUNT 30 

class SensorManager {
private:
    uint8_t _tdsPin;
    uint8_t _dhtPin;
    float _tdsValue;
    
    int _analogBuffer[SCOUNT];
    int _analogBufferIndex;
    unsigned long _analogSampleTimepoint;
    unsigned long _printTimepoint;

    DHT _dht; 

public:
    SensorManager(uint8_t tdsPin, uint8_t dhtPin);
    void begin();
    void update(); 
    float getTDS();
    float getTemperature();
    float getHumidity();
};

#endif