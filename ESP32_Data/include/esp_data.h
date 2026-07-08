#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>

struct TierData {
    float temp;
    int   pump;
    int   led;
    int   fan;
};

class esp_data {
public:
    esp_data();
    void begin(long baud);
    void update();
    void render();
private:
    float    _ph, _tempW, _tds;
    int      _pPeri;
    char     _time[9];
    TierData _tiers[3];

    String   _inBuffer;
    void     _parsePacket(const String& raw);
};