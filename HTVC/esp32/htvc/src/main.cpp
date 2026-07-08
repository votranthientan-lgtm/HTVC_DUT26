#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include "config.h"
#include "esp_data.h"
#include "mqtt_manager.h"
#include "wifi_manager.h"

// 1. Khai báo các đối tượng quản lý
static esp_data sensorData; 
static WifiManager wifiManager(htvc_config::WIFI_SSID, htvc_config::WIFI_PASSWORD);
static WiFiClientSecure wifiClient; 
static MqttManager mqttManager;

static unsigned long lastSensorUpdate = 0;
static unsigned long lastHeartbeat = 0;
static bool wifiReady = false;

namespace {
bool variantToBool(JsonVariantConst value) {
    if (value.is<bool>()) {
        return value.as<bool>();
    }
    if (value.is<int>()) {
        return value.as<int>() != 0;
    }
    if (value.is<float>()) {
        return value.as<float>() != 0.0f;
    }
    if (value.is<const char*>()) {
        String text = value.as<const char*>();
        text.trim();
        text.toLowerCase();
        return text == "1" || text == "true" || text == "on" || text == "yes";
    }
    return false;
}

const char* controlAckTopicForFloor(int floor) {
    switch (floor) {
        case 2: return htvc_config::TOPIC_CONTROL_ACK_T2;
        case 3: return htvc_config::TOPIC_CONTROL_ACK_T3;
        case 1:
        default:
            return htvc_config::TOPIC_CONTROL_ACK_T1;
    }
}

char controlCommandForFloor(int floor, const char* target, bool active) {
    if (target == nullptr) {
        return '\0';
    }

    if (strcmp(target, "fan") == 0) {
        switch (floor) {
            case 1: return active ? 'F' : 'f';
            case 2: return active ? 'G' : 'g';
            case 3: return active ? 'H' : 'h';
            default: return '\0';
        }
    }

    if (strcmp(target, "pump") == 0) {
        switch (floor) {
            case 1: return active ? 'P' : 'p';
            case 2: return active ? 'Q' : 'q';
            case 3: return active ? 'R' : 'r';
            default: return '\0';
        }
    }

    if (strcmp(target, "led") == 0) {
        switch (floor) {
            case 1: return active ? 'L' : 'l';
            case 2: return active ? 'M' : 'm';
            case 3: return active ? 'N' : 'n';
            default: return '\0';
        }
    }

    return '\0';
}

bool sendControlToMega(int floor, const char* target, JsonVariantConst value) {
    const bool active = variantToBool(value);
    const char command = controlCommandForFloor(floor, target, active);
    if (command == '\0') {
        Serial.printf("[MQTT] Unsupported control target=%s floor=%d\n", target != nullptr ? target : "<null>", floor);
        return false;
    }

    Serial2.write(static_cast<uint8_t>(command));
    Serial2.flush();
    Serial.printf("[UART->Mega] floor=%d target=%s value=%s cmd=%c\n",
                  floor,
                  target,
                  active ? "ON" : "OFF",
                  command);
    return true;
}
}  // namespace

// --- HÀM GỬI DỮ LIỆU ĐA TẦNG ---
static void publishTelemetrySnapshot(const char* source) {
    // Mảng các Topic tương ứng cho 3 tầng từ config.h
    const char* topics[] = {
        htvc_config::TOPIC_SENSORS_T1,
        htvc_config::TOPIC_SENSORS_T2,
        htvc_config::TOPIC_SENSORS_T3
    };

    Serial.println("\n[MQTT] Dang chuan bi gui du lieu 3 tang...");

    for (int i = 1; i <= 3; i++) {
        JsonDocument doc; 
        
        // Lấy dữ liệu của tầng i (1, 2, hoặc 3)
        sensorData.buildTelemetry(doc, i); 
        doc["event"] = (source != nullptr) ? source : "update";
        
        // Gửi lên Topic của tầng tương ứng
        if (mqttManager.publishJson(topics[i-1], doc)) {
            Serial.printf("   [+] Tang %d: Thanh cong\n", i);
        } else {
            Serial.printf("   [!] Tang %d: That bai (Buffer day/Mat ket noi)\n", i);
        }
        
        // Nghỉ ngắn để ESP32 kịp xử lý SSL cho gói tiếp theo
        delay(100); 
    }
}

// 2. Hàm callback nhận lệnh từ Web
void onControlMessage(int floor, const char* deviceId, const char* target, JsonVariantConst value, const char* commandId, bool shouldAck) {
    Serial.printf("\n[MQTT] Nhan lenh dieu khien: device=%s floor=%d target=%s\n",
                  deviceId != nullptr && deviceId[0] != '\0' ? deviceId : "<unknown>",
                  floor,
                  target != nullptr ? target : "<null>");

    const bool success = sendControlToMega(floor, target, value);
    if (success) {
        sensorData.applyControl(floor, target, value);
        if (shouldAck) {
            mqttManager.publishControlAck(controlAckTopicForFloor(floor), floor, deviceId, target, value, true, commandId);
        }
        Serial.println("-> Thuc thi va phan hoi thanh cong");
    } else {
        if (shouldAck) {
            mqttManager.publishControlAck(controlAckTopicForFloor(floor), floor, deviceId, target, value, false, commandId);
        }
        Serial.println("-> Lenh khong duoc ho tro hoac gui Mega that bai");
    }
}

// 3. Hàm Setup
void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n**************************************");
    Serial.println("* HTVC PROJECT - 3 FLOORS MONITORING *");
    Serial.println("**************************************");

    sensorData.begin(115200);
    wifiClient.setInsecure(); 

    Serial.println("[WiFi] Dang ket noi...");
    wifiManager.begin();
    wifiReady = true;

    Serial.println("[MQTT] Dang thiet lap ket noi HiveMQ Cloud...");
    mqttManager.begin(
        wifiClient,
        htvc_config::MQTT_BROKER_HOST,
        htvc_config::MQTT_BROKER_PORT,
        htvc_config::MQTT_CLIENT_ID,
        htvc_config::MQTT_USERNAME,
        htvc_config::MQTT_PASSWORD,
        htvc_config::TOPIC_CONTROL,
        onControlMessage
    );

    Serial.println("[SYSTEM] San sang! Dang cho du lieu tu Mega...");
}

// 4. Hàm Loop
void loop() {
    if (wifiReady) {
        wifiManager.ensureConnected();
    }
    mqttManager.loop();

    // Cập nhật dữ liệu từ Mega (UART)
    sensorData.update(); 

    // Gửi dữ liệu cảm biến định kỳ (mỗi 5 giây)
    if (millis() - lastSensorUpdate >= htvc_config::SENSOR_PUBLISH_INTERVAL_MS) {
        lastSensorUpdate = millis();
        if (mqttManager.isConnected()) {
            publishTelemetrySnapshot("sensor");
        }
    }

    // Gửi Heartbeat định kỳ
    if (millis() - lastHeartbeat >= htvc_config::HEARTBEAT_INTERVAL_MS) {
        lastHeartbeat = millis();
        JsonDocument hb;
        hb["deviceId"] = htvc_config::MQTT_CLIENT_ID;
        hb["status"] = "online";
        mqttManager.publishJson(htvc_config::TOPIC_HEARTBEAT, hb);
    }
}
