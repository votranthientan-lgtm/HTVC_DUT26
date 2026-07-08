#include "sensor_manager.h"

#define LED_PIN_1   2
#define LED_PIN_2   4
#define LED_PIN_3   16
#define PUMP_PIN_1  5
#define PUMP_PIN_2  17
#define PUMP_PIN_3  18

SensorManager::SensorManager()
    : _tds(0),
      _tempA(0),
      _hum(0),
      _ph(7.0f),
      _tempW(0),
      _pTuoi(0),
      _pAB(0),
      _pCD(0),
      _led(0),
      _currentTime("N/A"),
      _lastReceiveMs(0) {}

void SensorManager::begin(long baud) {
    Serial2.begin(baud, SERIAL_8N1, 16, 17);
    Serial1.begin(baud, SERIAL_8N1, 27, 26);

    pinMode(LED_PIN_1, OUTPUT);
    pinMode(LED_PIN_2, OUTPUT);
    pinMode(LED_PIN_3, OUTPUT);
    pinMode(PUMP_PIN_1, OUTPUT);
    pinMode(PUMP_PIN_2, OUTPUT);
    pinMode(PUMP_PIN_3, OUTPUT);

    digitalWrite(LED_PIN_1, LOW);
    digitalWrite(LED_PIN_2, LOW);
    digitalWrite(LED_PIN_3, LOW);
    digitalWrite(PUMP_PIN_1, LOW);
    digitalWrite(PUMP_PIN_2, LOW);
    digitalWrite(PUMP_PIN_3, LOW);
}

void SensorManager::setLED(int val) {
    _led = val ? 1 : 0;
    digitalWrite(LED_PIN_1, _led);
    digitalWrite(LED_PIN_2, _led);
    digitalWrite(LED_PIN_3, _led);
}

void SensorManager::setPumpTuoi(int val) {
    _pTuoi = val ? 1 : 0;
    digitalWrite(PUMP_PIN_1, _pTuoi);
}

bool SensorManager::update() {
    static String lineBuffer;
    bool updated = false;

    auto tryProcessLine = [&](String input, const char* portName) -> bool {
        input.trim();
        if (input.length() == 0) {
            return false;
        }

        const int dataIndex = input.indexOf("DATA:");
        if (dataIndex >= 0) {
            input = input.substring(dataIndex + 5);
            input.trim();
        } else if (!input.startsWith("{")) {
            Serial.print(F("UART RAW ["));
            Serial.print(portName);
            Serial.print(F("]: "));
            Serial.println(input);
            return false;
        }

        JsonDocument doc;
        const DeserializationError error = deserializeJson(doc, input);

        if (!error) {
            _currentTime = doc["time"].as<String>();
            _tds = doc["tds"].as<float>();
            _tempA = doc["tempA"].as<float>();
            _hum = doc["hum"].as<float>();
            _ph = doc["ph"].as<float>();
            _tempW = doc["tempW"].as<float>();
            _pTuoi = doc["pTuoi"].as<int>();
            _pAB = doc["pAB"].as<int>();
            _pCD = doc["pCD"].as<int>();
            _led = doc["led"].as<int>();
            _lastReceiveMs = millis();
            render();
            return true;
        }

        Serial.print(F("UART Error ["));
        Serial.print(portName);
        Serial.print(F("]: "));
        Serial.println(error.f_str());
        return false;
    };

    auto drainSerial = [&](HardwareSerial& serialPort, const char* portName) {
        while (serialPort.available() > 0) {
            const char c = static_cast<char>(serialPort.read());

            if (c == '\r') {
                continue;
            }

            if (c == '\n') {
                const String input = lineBuffer;
                lineBuffer = "";
                if (tryProcessLine(input, portName)) {
                    updated = true;
                }
                continue;
            }

            lineBuffer += c;
            if (lineBuffer.length() > 512) {
                lineBuffer = "";
            }
        }
    };

    drainSerial(Serial2, "S2");
    drainSerial(Serial1, "S1");

    return updated;
}

void SensorManager::render() {
    Serial.println(F("\n====== HTVC MONITORING ======"));
    Serial.print(F("Time  : "));
    Serial.println(_currentTime);
    Serial.print(F("Air   : "));
    Serial.print(_tempA, 1);
    Serial.print(F("C | "));
    Serial.print(_hum, 1);
    Serial.println(F("%"));
    Serial.print(F("Water : "));
    Serial.print(_tempW, 1);
    Serial.print(F("C | pH: "));
    Serial.println(_ph, 2);
    Serial.print(F("TDS   : "));
    Serial.print(_tds, 0);
    Serial.println(F(" ppm"));
    Serial.print(F("Pump  : "));
    Serial.print(_pTuoi ? "ON " : "OFF");
    Serial.print(F(" | LED: "));
    Serial.println(_led ? "ON" : "OFF");
    Serial.println(F("============================="));
}
