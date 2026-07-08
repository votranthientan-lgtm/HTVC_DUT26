#pragma once

#include <Arduino.h>

namespace htvc_config {
    // --- CẤU HÌNH WIFI ---
    static constexpr const char* WIFI_SSID = "@PHICOMM_09";
    static constexpr const char* WIFI_PASSWORD = "12345678";

    // --- CẤU HÌNH HIVEMQ CLOUD ---
    static constexpr const char* MQTT_BROKER_HOST = "e898a641a7d3497aa75239be5bb34cd3.s1.eu.hivemq.cloud";
    static constexpr int MQTT_BROKER_PORT = 8883;
    static constexpr const char* MQTT_USERNAME = "HTVC_TEST";
    static constexpr const char* MQTT_PASSWORD = "Turauthongminh1";
    static constexpr const char* MQTT_CLIENT_ID = "htvc-esp32-01";

    // --- CẤU HÌNH TOPIC ---
    static constexpr const char* TOPIC_SENSORS = "htvc/sensors/floor1";
    static constexpr const char* TOPIC_CONTROL = "htvc/control/#";
    static constexpr const char* TOPIC_CONTROL_ACK = "htvc/control/ack/floor1";
    static constexpr const char* TOPIC_HEARTBEAT = "htvc/heartbeat/floor1";

    // Topics cho 3 tầng (nếu mở rộng)
    static constexpr const char* TOPIC_SENSORS_T1 = "htvc/sensors/floor1";
    static constexpr const char* TOPIC_SENSORS_T2 = "htvc/sensors/floor2";
    static constexpr const char* TOPIC_SENSORS_T3 = "htvc/sensors/floor3";

    static constexpr const char* TOPIC_CONTROL_T1 = "htvc/control/floor1";
    static constexpr const char* TOPIC_CONTROL_T2 = "htvc/control/floor2";
    static constexpr const char* TOPIC_CONTROL_T3 = "htvc/control/floor3";

    static constexpr const char* TOPIC_CONTROL_ACK_T1 = "htvc/control/ack/floor1";
    static constexpr const char* TOPIC_CONTROL_ACK_T2 = "htvc/control/ack/floor2";
    static constexpr const char* TOPIC_CONTROL_ACK_T3 = "htvc/control/ack/floor3";

    static constexpr const char* DEVICE_ID_T1 = "htvc-floor1-001";
    static constexpr const char* DEVICE_ID_T2 = "htvc-floor2-001";
    static constexpr const char* DEVICE_ID_T3 = "htvc-floor3-001";

    // --- CHU KỲ GỬI (ms) ---
    static constexpr unsigned long SENSOR_PUBLISH_INTERVAL_MS = 5000UL;
    static constexpr unsigned long HEARTBEAT_INTERVAL_MS = 30000UL;

    // --- CẤU HÌNH CHÂN (PINS) ---
    static constexpr int LED_PIN_1 = 2;
    static constexpr int LED_PIN_2 = 4;
    static constexpr int LED_PIN_3 = 25;
    static constexpr int PUMP_PIN_1 = 5;
    static constexpr int PUMP_PIN_2 = 26;
    static constexpr int PUMP_PIN_3 = 18;

    static constexpr int SERIAL2_RX = 16;
    static constexpr int SERIAL2_TX = 17;
}

// Cấu trúc lưu trữ dữ liệu từng tầng
struct FloorData {
    float tds = 0;
    float tempA = 0;
    float hum = 0;
    float ph = 7.0;
    float tempW = 0;
    int pTuoi = 0;
    int pAB = 0;
    int pCD = 0;
    int fan = 0;
    int led = 0;
    int brightness = 0;
    String time = "N/A";
};
