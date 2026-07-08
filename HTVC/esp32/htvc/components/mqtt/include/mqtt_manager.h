#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>

class MqttManager {
public:
    using ControlHandler = void (*)(int floor,
                                    const char* deviceId,
                                    const char* target,
                                    JsonVariantConst value,
                                    const char* commandId,
                                    bool shouldAck);

    MqttManager();

    void begin(Client& netClient,
               const char* host,
               int port,
               const char* clientId,
               const char* username,
               const char* password,
               const char* controlTopic,
               ControlHandler handler);

    void loop();
    bool isConnected();

    bool publishJson(const char* topic, const JsonDocument& doc);
    bool publishControlAck(const char* topic,
                           int floor,
                           const char* deviceId,
                           const char* target,
                           JsonVariantConst value,
                           bool success = true,
                           const char* commandId = nullptr);

private:
    static MqttManager* _instance;
    PubSubClient _client;

    const char* _clientId;
    const char* _username;
    const char* _password;
    const char* _controlTopic;
    ControlHandler _controlHandler;
    String _lastPublishedAckCommandId;
    unsigned long _lastPublishedAckMs;

    void reconnect();
    void handleMessage(char* topic, byte* payload, unsigned int length);
    static void staticCallback(char* topic, byte* payload, unsigned int length);
};
