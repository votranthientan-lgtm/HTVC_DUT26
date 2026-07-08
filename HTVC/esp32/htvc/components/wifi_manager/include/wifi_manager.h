#pragma once

#include <Arduino.h>

void wifi_init(const char* ssid, const char* password);
bool wifi_wait_for_connection(unsigned long timeout_ms = 15000UL);
bool wifi_is_connected();
const char* wifi_get_ip_address();
int8_t wifi_get_rssi();
