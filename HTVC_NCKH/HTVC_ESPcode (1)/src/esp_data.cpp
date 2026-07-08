#include "esp_data.h"

esp_data::esp_data() {
    _ph = 0; _tempW = 0; _tds = 0; _pPeri = 0;
    strlcpy(_time, "00:00:00", sizeof(_time));
    for (int i = 0; i < 3; i++) _tiers[i] = {0, 0, 0, 0};
    _inBuffer.reserve(600);
}

void esp_data::begin(long baud) {
    Serial2.begin(baud, SERIAL_8N1, 16, 17);
}

void esp_data::update() {
    while (Serial2.available()) {
        char c = (char)Serial2.read();
        if (c == '\n') {
            _inBuffer.trim();
            if (_inBuffer.length() > 0) {
                _parsePacket(_inBuffer);
            }
            _inBuffer = "";
        } else {
            if (_inBuffer.length() < 600) {
                _inBuffer += c;
            } else {
                _inBuffer = ""; // xả nếu gói lỗi / quá dài
            }
        }
    }
}

void esp_data::_parsePacket(const String& raw) {
    if (raw.length() < 5 || raw[0] != '{') return;

    StaticJsonDocument<512> doc;
    DeserializationError err = deserializeJson(doc, raw);

    if (err) {
        Serial.print(F("[ESP] JSON loi: "));
        Serial.println(err.c_str());
        return;
    }

    // Thông số chung
    strlcpy(_time, doc["time"] | "00:00:00", sizeof(_time));
    _ph    = doc["ph"]   | 0.0f;
    _tempW = doc["tw"]   | 0.0f;
    _tds   = doc["tds"]  | 0.0f;
    _pPeri = doc["peri"] | 0;

    // 3 tầng
    JsonArray floors = doc["floors"];
    int i = 0;
    for (JsonObject f : floors) {
        if (i >= 3) break;
        _tiers[i].temp = f["temp"] | 0.0f;
        _tiers[i].pump = f["pump"] | 0;
        _tiers[i].led  = f["led"]  | 0;
        _tiers[i].fan  = f["fan"]  | 0;
        i++;
    }

    render();
}

void esp_data::render() {
    Serial.println(F("\n====== MONITORING (FROM MEGA) ======"));

    Serial.print(F("Gio  : ")); Serial.println(_time);
    Serial.print(F("Nuoc : ")); Serial.print(_tempW, 1); 
    Serial.print(F("C | pH: ")); Serial.println(_ph, 2);
    Serial.print(F("TDS  : ")); Serial.print(_tds, 0); Serial.println(F(" ppm"));
    Serial.print(F("B.Nhu dong: ")); Serial.println(_pPeri ? "ON" : "OFF");

    Serial.println(F("--- Trang thai 3 Tang ---"));
    for (int i = 0; i < 3; i++) {
        Serial.printf("T%d - Nhiet do: %.1fC | Bom: %s | LED: %s | Quat: %s\n",
            i + 1,
            _tiers[i].temp,
            _tiers[i].pump ? "ON " : "OFF",
            _tiers[i].led  ? "ON " : "OFF",
            _tiers[i].fan  ? "ON " : "OFF"
        );
    }
    Serial.println(F("===================================="));
}