#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "config.h"

class esp_data {
public:
    esp_data();

    void begin(long baud = 115200);
    bool update();
    void render() const;

    const FloorData& getFloorData(int floor) const;
    
    // Getters cho tầng 1
    float getTds() const { return _floors[0].tds; }
    float getTempAir() const { return _floors[0].tempA; }
    float getHumidity() const { return _floors[0].hum; }
    float getPH() const { return _floors[0].ph; }
    float getTempWater() const { return _floors[0].tempW; }
    int getPumpTuoi() const { return _floors[0].pTuoi; }
    int getPumpAB() const { return _floors[0].pAB; }
    int getPumpCD() const { return _floors[0].pCD; }
    int getLED() const { return _floors[0].led; }
    int getBrightness() const { return _floors[0].brightness; }
    String getTime() const { return _floors[0].time; }
    bool isConnected() const { return (millis() - _lastReceiveMs) < 10000UL; }

    // Setters & Điều khiển
    void setLED(int val);
    void setPumpTuoi(int val);
    void setPumpAB(int val);
    void setPumpCD(int val);
    void setBrightness(int value);
    bool applyControl(int floor, const char* target, JsonVariantConst value);

    // Đóng gói JSON gửi MQTT
    void buildTelemetry(JsonDocument& doc, int floor = 1) const;
    void buildStatus(JsonDocument& doc, const char* source = nullptr, int floor = 1) const;

private:
    void applyOutputs() const;
    bool parseLine(String input, const char* portName);
    bool parseMultiFloor(String jsonString);
    bool parseAlgorithmPacket(String jsonString, const char* portName);

    FloorData _floors[3];  
    int _peri;
    unsigned long _lastReceiveMs;
};