#include "mqtt_manager.h"

namespace {
String payloadToString(const byte* payload, unsigned int length) {
    String text;
    text.reserve(length + 1);
    for (unsigned int i = 0; i < length; ++i) {
        text += static_cast<char>(payload[i]);
    }
    text.trim();
    return text;
}

String normalizePythonDictPayload(String text) {
    text.trim();

    // Chuyển dạng Python repr/dict sang JSON hợp lệ.
    // Ví dụ:
    // {'deviceId': 'htvc-floor3-001', 'target': 'pump', 'value': True}
    // -> {"deviceId": "htvc-floor3-001", "target": "pump", "value": true}
    text.replace("'", "\"");
    text.replace("True", "true");
    text.replace("False", "false");
    text.replace("None", "null");
    return text;
}

bool isEncodedControlCommand(const String& raw) {
    return raw.startsWith("HTVCCTRL|");
}

bool decodeEncodedControlCommand(const String& raw, JsonDocument& doc) {
    if (!isEncodedControlCommand(raw)) {
        return false;
    }

    doc.clear();
    doc["type"] = "control_command";

    int start = raw.indexOf('|') + 1;
    while (start > 0 && start < static_cast<int>(raw.length())) {
        int end = raw.indexOf('|', start);
        if (end < 0) {
            end = raw.length();
        }

        String token = raw.substring(start, end);
        int sep = token.indexOf('=');
        if (sep > 0) {
            String key = token.substring(0, sep);
            String value = token.substring(sep + 1);
            key.trim();
            value.trim();

            if (key == "floor") {
                doc[key] = value.toInt();
            } else if (key == "value") {
                String lowered = value;
                lowered.toLowerCase();
                if (lowered == "1" || lowered == "true" || lowered == "on" || lowered == "yes") {
                    doc[key] = true;
                } else if (lowered == "0" || lowered == "false" || lowered == "off" || lowered == "no") {
                    doc[key] = false;
                } else {
                    char* endPtr = nullptr;
                    const double numeric = strtod(value.c_str(), &endPtr);
                    if (endPtr != value.c_str() && *endPtr == '\0') {
                        if (value.indexOf('.') >= 0) {
                            doc[key] = numeric;
                        } else {
                            doc[key] = static_cast<int>(numeric);
                        }
                    } else {
                        doc[key] = value;
                    }
                }
            } else {
                doc[key] = value;
            }
        }

        start = end + 1;
    }

    return !doc["target"].isNull() && !doc["value"].isNull();
}

bool parseCommandDocument(const String& raw, JsonDocument& doc) {
    DeserializationError error = deserializeJson(doc, raw);
    if (!error) {
        return true;
    }

    if (decodeEncodedControlCommand(raw, doc)) {
        return true;
    }

    String normalized = normalizePythonDictPayload(raw);
    if (normalized != raw) {
        doc.clear();
        error = deserializeJson(doc, normalized);
        if (!error) {
            return true;
        }
    }

    return false;
}

int floorFromTopic(const char* topic) {
    if (topic == nullptr) {
        return 1;
    }

    const char* floorText = strstr(topic, "floor");
    if (floorText == nullptr) {
        return 1;
    }

    const int floor = atoi(floorText + 5);
    return (floor >= 1 && floor <= 3) ? floor : 1;
}
}  // namespace

MqttManager* MqttManager::_instance = nullptr;

MqttManager::MqttManager()
    : _client(),
      _clientId(nullptr),
      _username(nullptr),
      _password(nullptr),
      _controlTopic(nullptr),
      _controlHandler(nullptr),
      _lastPublishedAckCommandId(),
      _lastPublishedAckMs(0) {}

void MqttManager::begin(Client& netClient,
                        const char* host,
                        int port,
                        const char* clientId,
                        const char* username,
                        const char* password,
                        const char* controlTopic,
                        ControlHandler handler) {
    _instance = this;
    _clientId = clientId;
    _username = username;
    _password = password;
    _controlTopic = controlTopic;
    _controlHandler = handler;

    _client.setClient(netClient);
    _client.setServer(host, port);
    _client.setCallback(MqttManager::staticCallback);
    _client.setBufferSize(1024);
}

void MqttManager::loop() {
    if (!_client.connected()) {
        reconnect();
    }
    _client.loop();
}

bool MqttManager::isConnected() {
    return _client.connected();
}

void MqttManager::reconnect() {
    int attempt = 0;
    while (!_client.connected()) {
        attempt++;
        Serial.print("[MQTT] Attempting to connect (attempt ");
        Serial.print(attempt);
        Serial.println(")...");
        
        if (_client.connect(_clientId, _username, _password)) {
            Serial.println("[MQTT] ✓ Connected successfully!");
            if (_controlTopic != nullptr) {
                Serial.print("[MQTT] Subscribing to: ");
                Serial.println(_controlTopic);
                _client.subscribe(_controlTopic);
            }
        } else {
            int rc = _client.state();
            Serial.print("[MQTT] ✗ Connection failed (code: ");
            Serial.print(rc);
            Serial.println("). Retrying in 3s...");
            delay(3000);
        }
    }
}

bool MqttManager::publishJson(const char* topic, const JsonDocument& doc) {
    char buffer[768];
    const size_t len = serializeJson(doc, buffer, sizeof(buffer));
    if (len == 0) {
        return false;
    }
    return _client.publish(topic, buffer);
}

bool MqttManager::publishControlAck(const char* topic,
                                    int floor,
                                    const char* deviceId,
                                    const char* target,
                                    JsonVariantConst value,
                                    bool success,
                                    const char* commandId) {
    JsonDocument ack;
    ack["type"] = "control_ack";
    ack["floor"] = floor;
    if (deviceId != nullptr && deviceId[0] != '\0') {
        ack["deviceId"] = deviceId;
    }
    ack["target"] = target;
    ack["value"] = value;
    ack["success"] = success;
    if (commandId != nullptr && commandId[0] != '\0') {
        ack["commandId"] = commandId;
        _lastPublishedAckCommandId = commandId;
        _lastPublishedAckMs = millis();
    }
    return publishJson(topic, ack);
}

void MqttManager::staticCallback(char* topic, byte* payload, unsigned int length) {
    if (_instance != nullptr) {
        _instance->handleMessage(topic, payload, length);
    }
}

void MqttManager::handleMessage(char* topic, byte* payload, unsigned int length) {
    JsonDocument doc;
    const String rawPayload = payloadToString(payload, length);
    Serial.print(F("[MQTT] RX "));
    Serial.print(topic);
    Serial.print(F(": "));
    Serial.println(rawPayload);

    if (!parseCommandDocument(rawPayload, doc)) {
        Serial.println(F("[MQTT] Parse command failed"));
        return;
    }

    const char* messageType = doc["type"] | "";
    const bool isControlAck = strcmp(messageType, "control_ack") == 0;
    if (isControlAck && doc["success"].is<bool>() && !doc["success"].as<bool>()) {
        Serial.println(F("[MQTT] Ignoring failed control_ack"));
        return;
    }

    const char* commandId = doc["commandId"] | "";
    if (isControlAck &&
        commandId[0] != '\0' &&
        _lastPublishedAckCommandId == commandId &&
        millis() - _lastPublishedAckMs < 3000UL) {
        Serial.println(F("[MQTT] Ignoring self-published control_ack"));
        return;
    }

    const char* target = doc["target"];
    if (target == nullptr || _controlHandler == nullptr) {
        Serial.println(F("[MQTT] Missing target or handler"));
        return;
    }

    const int floor = doc["floor"] | floorFromTopic(topic);
    const char* deviceId = doc["deviceId"] | "";
    Serial.print(F("[MQTT] Decoded target="));
    Serial.print(target);
    Serial.print(F(" type="));
    Serial.print(messageType);
    Serial.print(F(" commandId="));
    Serial.println(commandId);
    _controlHandler(floor, deviceId, target, doc["value"], commandId, !isControlAck);
}
