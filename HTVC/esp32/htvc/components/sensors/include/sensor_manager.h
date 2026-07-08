#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

class SensorManager {
public:
    SensorManager();

    void begin(long baud = 115200);
    bool update();

    float getTds() const { return _tds; }
    float getTempAir() const { return _tempA; }
    float getHumidity() const { return _hum; }
    float getPH() const { return _ph; }
    float getTempWater() const { return _tempW; }
    int getPumpTuoi() const { return _pTuoi; }
    int getPumpAB() const { return _pAB; }
    int getPumpCD() const { return _pCD; }
    int getLED() const { return _led; }
    String getTime() const { return _currentTime; }
    bool isConnected() const { return (millis() - _lastReceiveMs) < 10000; }

    void setLED(int val);
    void setPumpTuoi(int val);

private:
    void render();

    float _tds;
    float _tempA;
    float _hum;
    float _ph;
    float _tempW;
    int _pTuoi;
    int _pAB;
    int _pCD;
    int _led;
    String _currentTime;
    unsigned long _lastReceiveMs;
};
