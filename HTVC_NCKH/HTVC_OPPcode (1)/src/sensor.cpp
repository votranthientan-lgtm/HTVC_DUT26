#include "sensor.h"

SensorManager::SensorManager(uint8_t tdsPin, uint8_t dhtPin)
    : _dht(dhtPin, 11) { 
    _tdsPin = tdsPin;
    _dhtPin = dhtPin;
    _tdsValue = 0;
    _analogBufferIndex = 0;
    _analogSampleTimepoint = millis();
    _printTimepoint = millis();
    for(int i = 0; i < SCOUNT; i++) _analogBuffer[i] = 0;
}

void SensorManager::begin() {
    pinMode(_tdsPin, INPUT);
    _dht.begin();
}

void SensorManager::update() {
    if (millis() - _analogSampleTimepoint > 40U) {
        _analogSampleTimepoint = millis();
        _analogBuffer[_analogBufferIndex] = analogRead(_tdsPin);
        _analogBufferIndex++;
        if (_analogBufferIndex >= SCOUNT) _analogBufferIndex = 0;
    }

    if (millis() - _printTimepoint > 800U) {
        _printTimepoint = millis();
        
        float sum = 0;
        int count = 0;
        for (int i = 0; i < SCOUNT; i++) {
            if (_analogBuffer[i] > 0) { 
                sum += (float)_analogBuffer[i];
                count++;
            }
        }
        
        if (count > 0) {
            float averageVoltage = (sum / (float)count) * 5.0 / 1024.0;
            
            // TỰ ĐỘNG lấy nhiệt độ từ DHT của tầng này để bù trừ (Thay vì ép 25.0)
            float temperature = getTemperature(); 

            float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
            float compensationVolatge = averageVoltage / compensationCoefficient;

            // Công thức bậc 3 giữ nguyên của ông
            float term1 = 133.42 * pow(compensationVolatge, 3);
            float term2 = 255.86 * pow(compensationVolatge, 2);
            float term3 = 857.39 * compensationVolatge;

            _tdsValue = (term1 - term2 + term3) * 0.5;
        }
    }
}

float SensorManager::getTDS() { return _tdsValue; }

float SensorManager::getTemperature() {
    float t = _dht.readTemperature();
    return (isnan(t)) ? 25.0 : t; // Giữ nguyên logic trả về 25 nếu lỗi
}

float SensorManager::getHumidity() {
    float h = _dht.readHumidity();
    return isnan(h) ? 0.0 : h;
}