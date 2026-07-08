#pragma once

namespace htvc_config {

// WiFi settings
static constexpr const char* WIFI_SSID = "@PHICOMM_09";
static constexpr const char* WIFI_PASSWORD = "12345678";

// MQTT broker settings
static constexpr const char* MQTT_BROKER_HOST = "e898a641a7d3497aa75239be5bb34cd3.s1.eu.hivemq.cloud";
static constexpr int MQTT_BROKER_PORT = 8883;
static constexpr const char* MQTT_USERNAME = "HTVC_WEB";
static constexpr const char* MQTT_PASSWORD = "Turauthongminh1";
static constexpr const char* MQTT_CLIENT_ID = "htvc-esp32-01";

// === Device IDs (3 floors) ===
static constexpr const char* DEVICE_ID_T1 = "htvc-floor1-001";
static constexpr const char* DEVICE_ID_T2 = "htvc-floor2-001";
static constexpr const char* DEVICE_ID_T3 = "htvc-floor3-001";
static constexpr const char* DEVICE_ID = DEVICE_ID_T1; // Default for backward compat

// === Device Names (3 floors) ===
static constexpr const char* DEVICE_NAME_T1 = "Tu Rau Thong Minh - Tang 1";
static constexpr const char* DEVICE_NAME_T2 = "Tu Rau Thong Minh - Tang 2";
static constexpr const char* DEVICE_NAME_T3 = "Tu Rau Thong Minh - Tang 3";
static constexpr const char* DEVICE_NAME = DEVICE_NAME_T1; // Default for backward compat

// === MQTT Sensor Topics (3 floors) ===
static constexpr const char* TOPIC_SENSORS_T1 = "htvc/sensors/floor1";
static constexpr const char* TOPIC_SENSORS_T2 = "htvc/sensors/floor2";
static constexpr const char* TOPIC_SENSORS_T3 = "htvc/sensors/floor3";
static constexpr const char* TOPIC_SENSORS = TOPIC_SENSORS_T1; // Default

// === MQTT Control Topics ===
static constexpr const char* TOPIC_CONTROL = "htvc/control/+";
static constexpr const char* TOPIC_CONTROL_T1 = "htvc/control/floor1";
static constexpr const char* TOPIC_CONTROL_T2 = "htvc/control/floor2";
static constexpr const char* TOPIC_CONTROL_T3 = "htvc/control/floor3";

// === MQTT Control Acknowledgment Topics ===
static constexpr const char* TOPIC_CONTROL_ACK = "htvc/control/ack/floor1";
static constexpr const char* TOPIC_CONTROL_ACK_T1 = "htvc/control/ack/floor1";
static constexpr const char* TOPIC_CONTROL_ACK_T2 = "htvc/control/ack/floor2";
static constexpr const char* TOPIC_CONTROL_ACK_T3 = "htvc/control/ack/floor3";

// === MQTT Heartbeat Topics (3 floors) ===
static constexpr const char* TOPIC_HEARTBEAT_T1 = "htvc/heartbeat/floor1";
static constexpr const char* TOPIC_HEARTBEAT_T2 = "htvc/heartbeat/floor2";
static constexpr const char* TOPIC_HEARTBEAT_T3 = "htvc/heartbeat/floor3";
static constexpr const char* TOPIC_HEARTBEAT = TOPIC_HEARTBEAT_T1; // Default

// === GPIO Pins ===
static constexpr int LED_PIN_1 = 2;
static constexpr int LED_PIN_2 = 4;
static constexpr int LED_PIN_3 = 25;
static constexpr int PUMP_PIN_1 = 5;
static constexpr int PUMP_PIN_2 = 26;
static constexpr int PUMP_PIN_3 = 18;

// === Serial Configuration ===
static constexpr int SERIAL2_BAUD = 115200;
static constexpr int SERIAL2_RX = 16;
static constexpr int SERIAL2_TX = 17;

// === Publish intervals ===
static constexpr unsigned long SENSOR_PUBLISH_INTERVAL_MS = 5000UL;   // 5 seconds
static constexpr unsigned long HEARTBEAT_INTERVAL_MS = 30000UL;       // 30 seconds

}  // namespace htvc_config
