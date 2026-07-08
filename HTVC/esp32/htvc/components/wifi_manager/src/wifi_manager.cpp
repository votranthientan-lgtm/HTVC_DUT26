#include "../include/wifi_manager.h"

#include <WiFi.h>

static const char* s_ssid = nullptr;
static const char* s_password = nullptr;
static String s_ip_cache;

void wifi_init(const char* ssid, const char* password) {
    s_ssid = ssid;
    s_password = password;

    WiFi.mode(WIFI_STA);
    if (s_ssid != nullptr) {
        WiFi.begin(s_ssid, s_password);
    }
}

bool wifi_wait_for_connection(unsigned long timeout_ms) {
    if (WiFi.status() == WL_CONNECTED) {
        return true;
    }

    if (s_ssid == nullptr) {
        return false;
    }

    WiFi.disconnect(true);
    delay(100);
    WiFi.begin(s_ssid, s_password);

    const unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeout_ms) {
        delay(250);
    }

    return WiFi.status() == WL_CONNECTED;
}

bool wifi_is_connected() {
    return WiFi.status() == WL_CONNECTED;
}

const char* wifi_get_ip_address() {
    if (!wifi_is_connected()) {
        return "0.0.0.0";
    }

    s_ip_cache = WiFi.localIP().toString();
    return s_ip_cache.c_str();
}

int8_t wifi_get_rssi() {
    if (!wifi_is_connected()) {
        return -127;
    }
    return static_cast<int8_t>(WiFi.RSSI());
}
