# HTVC - Tủ Rau Thông Minh

## ESP32 Firmware v2.0

### Thư viện cần cài đặt (Arduino IDE / PlatformIO)

```
- WiFi.h (built-in)
- PubSubClient.h (by Nick O'Leary)
- ArduinoJson.h (by Benoit Blanchon)
- WebSocketsClient.h (by Links2004) - optional backup
```

### Cài đặt PubSubClient

**Arduino IDE:**
1. Sketch > Include Library > Manage Libraries
2. Tìm "PubSubClient" by Nick O'Leary
3. Cài đặt phiên bản mới nhất

**PlatformIO (platformio.ini):**
```ini
lib_deps = 
    pubsubclient
    arduinojson
    links2004/WebSockets@^2.3.7
```

### Cấu hình

Chỉnh sửa `src/main.cpp`:

```cpp
// WiFi
const char* WIFI_SSID = "Your_WiFi_SSID";
const char* WIFI_PASSWORD = "Your_WiFi_Password";

// HiveMQ Cloud
const char* MQTT_BROKER_HOST = "e898a641a7d3497aa75239be5bb34cd3.s1.eu.hivemq.cloud";
const int   MQTT_BROKER_PORT = 8883;
const char* MQTT_USERNAME = "HTVC_WEB";
const char* MQTT_PASSWORD = "Turauthongminh1";
const char* MQTT_CLIENT_ID = "htvc-esp32-01";
```

### Các topic MQTT

| Topic | Chiều | Mô tả |
|-------|-------|-------|
| `htvc/sensors/floor1` | ESP32 → Server | Dữ liệu cảm biến |
| `htvc/control/floor1` | Server → ESP32 | Lệnh điều khiển |
| `htvc/heartbeat/floor1` | ESP32 → Server | Heartbeat |

### Lệnh điều khiển (JSON)

```json
// Bật LED
{"target": "led", "value": true}

// Tắt LED
{"target": "led", "value": false}

// Bật bơm
{"target": "pump", "value": true}

// Điều chỉnh độ sáng LED (0-100)
{"target": "brightness", "value": 75}
```

### Payload cảm biến (ESP32 gửi)

```json
{
  "deviceId": "htvc-floor1-001",
  "deviceName": "Tủ Rau Thông Minh - Tầng 1",
  "timestamp": 1234567890,
  "ip": "192.168.1.100",
  "rssi": -45,
  "data": {
    "tempAir": 25.5,
    "humidity": 60.2,
    "tds": 450,
    "ph": 6.5,
    "tempWater": 24.0,
    "pump": 0,
    "led": 0,
    "ledBrightness": 100,
    "connected": true
  }
}
```

### Build & Upload

**Arduino IDE:**
1. Chọn Board: ESP32 Dev Module
2. Configure:
   - Upload Speed: 115200
   - Flash Size: 4MB
   - Partition Scheme: Default
3. Upload

**PlatformIO:**
```bash
cd esp32/htvc
pio run --target upload
pio monitor --baud 115200
```

### Kiểm tra

1. Mở Serial Monitor (115200 baud)
2. ESP32 sẽ kết nối WiFi → HiveMQ Cloud
3. Gửi dữ liệu cảm biến mỗi 10 giây
4. Nhận lệnh điều khiển từ server

### Troubleshooting

**MQTT Connection Failed:**
- Kiểm tra username/password HiveMQ Cloud
- Kiểm tra network/firewall
- Đảm bảo port 8883 mở

**No data from Mega:**
- ESP32 chờ dữ liệu từ Arduino Mega qua UART
- Nếu không có Mega, dữ liệu sẽ được simulate
