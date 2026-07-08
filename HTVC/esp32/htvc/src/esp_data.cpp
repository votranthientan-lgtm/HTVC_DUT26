#include "esp_data.h"

namespace {
bool valueToBool(JsonVariantConst value) {
    if (value.is<bool>()) {
        return value.as<bool>();
    }
    if (value.is<int>()) {
        return value.as<int>() != 0;
    }
    if (value.is<float>()) {
        return value.as<float>() != 0.0f;
    }
    if (value.is<const char*>()) {
        String text = value.as<const char*>();
        text.trim();
        text.toLowerCase();
        return text == "1" || text == "true" || text == "on" || text == "yes";
    }
    return false;
}
}  // namespace

esp_data::esp_data() : _lastReceiveMs(0) {
    _peri = 0;
    for (int i = 0; i < 3; i++) {
        _floors[i].tds = 0;
        _floors[i].tempA = 0;
        _floors[i].hum = 0;
        _floors[i].ph = 7.0;
        _floors[i].tempW = 0;
        _floors[i].pTuoi = 0;
        _floors[i].pAB = 0;
        _floors[i].pCD = 0;
        _floors[i].fan = 0;
        _floors[i].led = 0;
        _floors[i].brightness = 0;
        _floors[i].time = "N/A";
    }
}

void esp_data::begin(long baud) {   
    Serial2.begin(baud, SERIAL_8N1, htvc_config::SERIAL2_RX, htvc_config::SERIAL2_TX); 

    pinMode(htvc_config::LED_PIN_1, OUTPUT);
    pinMode(htvc_config::LED_PIN_2, OUTPUT);
    pinMode(htvc_config::LED_PIN_3, OUTPUT);
    pinMode(htvc_config::PUMP_PIN_1, OUTPUT);
    pinMode(htvc_config::PUMP_PIN_2, OUTPUT);
    pinMode(htvc_config::PUMP_PIN_3, OUTPUT);
    applyOutputs();
}

bool esp_data::update() {
    if (Serial2.available() <= 0) {
        return false;
    }

    String rawInput = Serial2.readStringUntil('\n'); 
    rawInput.trim(); 
    return parseLine(rawInput, "Serial2");
}

bool esp_data::parseLine(String input, const char* portName) {
    input.trim();

    if (input.startsWith("{")) {
        return parseAlgorithmPacket(input, portName);
    }

    if (input.startsWith("DATA_MULTI:")) {
        String jsonPart = input.substring(11);
        jsonPart.trim();
        return parseMultiFloor(jsonPart);
    }

    if (input.startsWith("DATA:")) {
        String jsonPart = input.substring(5);
        jsonPart.trim();

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, jsonPart);

        if (error) {
            Serial.print(F("UART Error: "));
            Serial.println(error.f_str());
            return false;
        }

        _floors[0].time = doc["time"].as<String>();
        _floors[0].tds = doc["tds"].as<float>();
        _floors[0].tempA = doc["tempA"].as<float>();
        _floors[0].hum = doc["hum"].as<float>();
        _floors[0].ph = doc["ph"].as<float>();
        _floors[0].tempW = doc["tempW"].as<float>();
        _floors[0].pTuoi = doc["pTuoi"].as<int>();
        _floors[0].pAB = doc["pAB"].as<int>();
        _floors[0].pCD = doc["pCD"].as<int>();
        _floors[0].led = doc["led"].as<int>();
        _lastReceiveMs = millis();

        applyOutputs();
        render();
        return true;
    }
    return false;
}

bool esp_data::parseMultiFloor(String jsonString) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonString);

    if (error) return false;

    const char* floorKeys[] = {"t1", "t2", "t3"};
    for (int i = 0; i < 3; i++) {
        if (doc[floorKeys[i]].isNull()) continue;

        JsonObjectConst floorObj = doc[floorKeys[i]].as<JsonObjectConst>();
        _floors[i].time = floorObj["time"].as<String>();
        _floors[i].tds = floorObj["tds"].as<float>();
        _floors[i].tempA = floorObj["tempA"].as<float>();
        _floors[i].hum = floorObj["hum"].as<float>();
        _floors[i].ph = floorObj["ph"].as<float>();
        _floors[i].tempW = floorObj["tempW"].as<float>();
        _floors[i].pTuoi = floorObj["pTuoi"].as<int>();
        _floors[i].pAB = floorObj["pAB"].as<int>();
        _floors[i].pCD = floorObj["pCD"].as<int>();
        _floors[i].fan = floorObj["fan"].as<int>();
        _floors[i].led = floorObj["led"].as<int>();
    }

    _lastReceiveMs = millis();
    applyOutputs();
    render();
    return true;
}

bool esp_data::parseAlgorithmPacket(String jsonString, const char* portName) {
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonString);

    if (error) return false;

    _peri = doc["peri"] | 0;
    const String globalTime = doc["time"] | String("00:00:00");
    const float globalPh = doc["ph"] | 0.0f;
    const float globalTempW = doc["tw"] | doc["tempW"] | 0.0f;
    const float globalTds = doc["tds"] | 0.0f;

    _floors[0].time = globalTime;
    _floors[0].ph = globalPh;
    _floors[0].tempW = globalTempW;
    _floors[0].tds = globalTds;

    JsonArray floors = doc["floors"].as<JsonArray>();
    int i = 0;
    for (JsonObject floorObj : floors) {
        if (i >= 3) break;
        _floors[i].time = globalTime;
        _floors[i].tempA = floorObj["temp"] | floorObj["tempA"] | 0.0f;
        _floors[i].hum = floorObj["hum"] | floorObj["humidity"] | 0.0f;
        _floors[i].ph = globalPh;
        _floors[i].tempW = globalTempW;
        _floors[i].tds = globalTds;
        _floors[i].pTuoi = floorObj["pump"] | floorObj["pTuoi"] | 0;
        _floors[i].pAB = floorObj["pumpAB"] | floorObj["pAB"] | 0;
        _floors[i].pCD = floorObj["pumpCD"] | floorObj["pCD"] | 0;
        _floors[i].fan = floorObj["fan"] | 0;
        _floors[i].led = floorObj["led"] | 0;
        ++i;
    }

    _lastReceiveMs = millis();
    applyOutputs();
    render();
    return true;
}

void esp_data::render() const {
    Serial.println(F("\n====== MONITORING (FROM MEGA) ======"));
    for (int i = 0; i < 3; i++) {
        const FloorData& floor = _floors[i];
        Serial.print(F("--- FLOOR ")); Serial.print(i + 1); Serial.println(F(" ---"));
        Serial.print(F("Time: ")); Serial.println(floor.time);
        Serial.print(F("Air : ")); Serial.print(floor.tempA, 1); Serial.print(F("C | "));
        Serial.print(floor.hum, 1); Serial.println(F("%"));
        Serial.print(F("Water: ")); Serial.print(floor.tempW, 1); Serial.print(F("C | pH: "));
        Serial.println(floor.ph, 2);
        Serial.print(F("TDS : ")); Serial.print(floor.tds, 0); Serial.println(F(" ppm"));
    }
    Serial.println(F("===================================="));
}

void esp_data::applyOutputs() const {
    digitalWrite(htvc_config::LED_PIN_1, _floors[0].led ? HIGH : LOW);
    digitalWrite(htvc_config::LED_PIN_2, _floors[1].led ? HIGH : LOW);
    digitalWrite(htvc_config::LED_PIN_3, _floors[2].led ? HIGH : LOW);
    digitalWrite(htvc_config::PUMP_PIN_1, _floors[0].pTuoi ? HIGH : LOW);
    digitalWrite(htvc_config::PUMP_PIN_2, _floors[1].pTuoi ? HIGH : LOW);
    digitalWrite(htvc_config::PUMP_PIN_3, _floors[2].pTuoi ? HIGH : LOW);
}

void esp_data::buildTelemetry(JsonDocument& doc, int floor) const {
    if (floor < 1 || floor > 3) floor = 1;
    const FloorData& data = _floors[floor - 1];
    
    const char* deviceIds[] = { htvc_config::DEVICE_ID_T1, htvc_config::DEVICE_ID_T2, htvc_config::DEVICE_ID_T3 };
    const char* deviceNames[] = { "Tu Rau Thong Minh - Tang 1", "Tu Rau Thong Minh - Tang 2", "Tu Rau Thong Minh - Tang 3" };
    
    doc["deviceId"] = deviceIds[floor - 1];
    doc["deviceName"] = deviceNames[floor - 1];
    doc["floor"] = floor;
    doc["time"] = data.time;
    doc["tempAir"] = data.tempA;
    doc["humidity"] = data.hum;
    doc["tds"] = data.tds;
    doc["ph"] = data.ph;
    doc["tempWater"] = data.tempW;
    doc["pump"] = data.pTuoi;
    doc["pTuoi"] = data.pTuoi;
    doc["pAB"] = data.pAB;
    doc["pCD"] = data.pCD;
    doc["fan"] = data.fan;
    doc["led"] = data.led;
    doc["brightness"] = data.brightness;
    doc["peri"] = _peri;
    doc["online"] = isConnected();
}

void esp_data::buildStatus(JsonDocument& doc, const char* source, int floor) const {
    buildTelemetry(doc, floor);
    doc["source"] = source != nullptr ? source : "status";
}

const FloorData& esp_data::getFloorData(int floor) const {
    if (floor < 1 || floor > 3) floor = 1;
    return _floors[floor - 1];
}

bool esp_data::applyControl(int floor, const char* target, JsonVariantConst value) {
    if (target == nullptr || floor < 1 || floor > 3) return false;

    const bool active = valueToBool(value);
    FloorData& data = _floors[floor - 1];

    if (strcmp(target, "led") == 0) {
        data.led = active ? 1 : 0;
        applyOutputs();
        return true;
    }
    if (strcmp(target, "pump") == 0 || strcmp(target, "pumpTuoi") == 0) {
        data.pTuoi = active ? 1 : 0;
        applyOutputs();
        return true;
    }
    if (strcmp(target, "pumpAB") == 0) {
        data.pAB = active ? 1 : 0;
        applyOutputs();
        return true;
    }
    if (strcmp(target, "pumpCD") == 0) {
        data.pCD = active ? 1 : 0;
        applyOutputs();
        return true;
    }
    if (strcmp(target, "fan") == 0) {
        data.fan = active ? 1 : 0;
        return true;
    }
    if (strcmp(target, "brightness") == 0) {
        data.brightness = value.is<int>() ? constrain(value.as<int>(), 0, 255) : (active ? 255 : 0);
        applyOutputs();
        return true;
    }
    return false;
}
