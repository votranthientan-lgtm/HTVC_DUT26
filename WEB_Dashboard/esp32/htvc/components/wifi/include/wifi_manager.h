#pragma once

#include <WiFi.h>

class WifiManager {
public:
    WifiManager(const char* ssid, const char* password);

    void begin();
    bool ensureConnected(unsigned long timeoutMs = 15000);
    bool isConnected() const;
    IPAddress localIP() const;
    int32_t rssi() const;

private:
    const char* _ssid;
    const char* _password;
};
