#include "wifi_manager.h"

WifiManager::WifiManager(const char* ssid, const char* password)
    : _ssid(ssid), _password(password) {}

void WifiManager::begin() {
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);
}

bool WifiManager::ensureConnected(unsigned long timeoutMs) {
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

    WiFi.disconnect(true);
    delay(100);
    WiFi.begin(_ssid, _password);

    const unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeoutMs) {
        delay(250);
    }

    return WiFi.status() == WL_CONNECTED;
}

bool WifiManager::isConnected() const {
    return WiFi.status() == WL_CONNECTED;
}

IPAddress WifiManager::localIP() const {
    return WiFi.localIP();
}

int32_t WifiManager::rssi() const {
    return WiFi.RSSI();
}
