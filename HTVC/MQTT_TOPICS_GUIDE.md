# Hướng Dẫn MQTT Topics - HTVC Project

## 📡 Tổng Quan HiveMQ Cloud

**Broker:** `e898a641a7d3497aa75239be5bb34cd3.s1.eu.hivemq.cloud:8883`  
**Username:** `HTVC_WEB`  
**Password:** `Turauthongminh1`  
**Port:** `8883` (TLS/SSL)

---

## 🔄 Các Topic Chính

### 1️⃣ `htvc/sensors/floor1` - Dữ liệu cảm biến từ ESP32 → Web/Firebase

**Hướng:** ESP32 → Flask Server → Firebase  
**Tần suất:** Mỗi 5 giây  
**QoS:** 1  
**Mục đích:** Gửi dữ liệu cảm biến từ ESP32 lên cloud để web và Firebase nhận

#### Định dạng JSON:
```json
{
  "deviceId": "htvc-floor1-001",
  "deviceName": "Tu Rau Thong Minh - Tang 1",
  "source": "esp32",
  "status": "online",
  "event": "sensor",
  "data": {
    "time": "2026-04-28 14:30:45",
    "tempAir": 25.5,
    "humidity": 65.2,
    "ph": 6.8,
    "tds": 450,
    "tempWater": 24.3,
    "pump": 0,
    "pumpAB": 0,
    "pumpCD": 0,
    "led": 1,
    "ledBrightness": 100
  }
}
```

#### Cách sử dụng:
- **Flask Server** tự động lắng nghe và lưu vào Firebase Firestore
- **Web Dashboard** hiển thị dữ liệu real-time từ Firebase
- Bạn có thể test bằng MQTT Client (VD: MQTT Explorer):
  ```
  Subscribe: htvc/sensors/floor1
  ```

---

### 2️⃣ `htvc/control/floor1` - Lệnh điều khiển từ Web → ESP32

**Hướng:** Web → Flask Server → HiveMQ → ESP32  
**Kích hoạt:** Khi nhấn nút trên Dashboard  
**QoS:** 1  
**Mục đích:** Gửi lệnh bật/tắt LED, bơm từ web xuống ESP32

#### Định dạng JSON:
```json
{
  "target": "led",           // "led" | "pump" | "pump_ab" | "pump_cd" | "brightness" | "all_off"
  "value": true,             // true/false hoặc 0-100 cho brightness
  "commandId": "cmd_12345",  // ID duy nhất để theo dõi
  "deviceId": "htvc-floor1-001"
}
```

#### Ví dụ lệnh cụ thể:

**Bật LED:**
```json
{
  "target": "led",
  "value": true,
  "commandId": "cmd_001"
}
```

**Bật Bơm nước:**
```json
{
  "target": "pump",
  "value": true,
  "commandId": "cmd_002"
}
```

**Điều chỉnh độ sáng (50%):**
```json
{
  "target": "brightness",
  "value": 50,
  "commandId": "cmd_003"
}
```

**Tắt tất cả:**
```json
{
  "target": "all_off",
  "value": null,
  "commandId": "cmd_004"
}
```

#### Luồng thực thi:
1. Web gọi: `POST /api/dashboard/control`
2. Flask lưu lệnh vào Firebase + publish lên HiveMQ
3. ESP32 nhận từ `htvc/control/floor1`
4. ESP32 bật GPIO tương ứng
5. ESP32 gửi ACK vào `htvc/control/ack/floor1`
6. Flask nhận ACK từ `htvc/control/ack/floor1` (tùy chọn)

---

### 3️⃣ `htvc/control/ack/floor1` - Xác nhận lệnh từ ESP32 → Web/Server

**Hướng:** ESP32 → Flask Server → Web  
**Kích hoạt:** Sau khi ESP32 xử lý lệnh  
**QoS:** 1  
**Mục đích:** Xác nhận ESP32 đã nhận và thực thi lệnh

#### Định dạng JSON:
```json
{
  "deviceId": "htvc-floor1-001",
  "target": "led",
  "value": true,
  "success": true,
  "commandId": "cmd_001",
  "timestamp": "2026-04-28T14:30:50Z"
}
```

#### Cách sử dụng:
- Flask Server tự động lắng nghe (subscribe `htvc/control/ack/+`)
- Cập nhật trạng thái nút trên web nếu lệnh thành công
- Log các lệnh không thành công để debug

---

### 4️⃣ `htvc/status/floor1` - Trạng thái hiện tại của ESP32

**Hướng:** ESP32 → Server (on-demand)  
**Kích hoạt:** Khi ESP32 khởi động hoặc bị hỏi  
**QoS:** 1  
**Mục đích:** Lấy trạng thái tức thời (LED on/off, bơm on/off, v.v.)

#### Định dạng JSON:
```json
{
  "deviceId": "htvc-floor1-001",
  "deviceName": "Tu Rau Thong Minh - Tang 1",
  "status": "online",
  "tempAir": 25.5,
  "humidity": 65.2,
  "ph": 6.8,
  "tds": 450,
  "tempWater": 24.3,
  "pump": 0,
  "pumpAB": 0,
  "pumpCD": 0,
  "led": 1,
  "ledBrightness": 100,
  "time": "2026-04-28 14:30:45"
}
```

#### Cách sử dụng:
- Subscribe để lấy trạng thái real-time
- Dùng khi cần kiểm tra nhanh trạng thái thiết bị

---

### 5️⃣ `htvc/heartbeat/floor1` - Nhịp tim từ ESP32 (mỗi 30s)

**Hướng:** ESP32 → Server  
**Tần suất:** Mỗi 30 giây  
**QoS:** 1  
**Mục đích:** Báo ESP32 vẫn online, kiểm tra WiFi + MQTT

#### Định dạng JSON:
```json
{
  "deviceId": "htvc-floor1-001",
  "deviceName": "Tu Rau Thong Minh - Tang 1",
  "status": "online",
  "wifi": true,
  "mqtt": true,
  "timestamp": "2026-04-28T14:30:50Z"
}
```

#### Cách sử dụng:
- Flask Server lắng nghe để cập nhật `lastSeen` trong Firebase
- Nếu không nhận heartbeat >60s → thiết bị "offline"
- Giúp phát hiện kết nối bị mất

---

## 📊 Topic Subscription Map (Flask Server)

| Topic | Subscription | Xử lý |
|-------|--------------|--------|
| `htvc/sensors/+` | Subscribe | Lưu vào Firebase, gửi cho web |
| `htvc/control/+` | Subscribe (để lắng nghe phản hồi) | Publish lệnh xuống |
| `htvc/control/ack/+` | Subscribe | Cập nhật trạng thái lệnh |
| `htvc/status/+` | Subscribe | Lưu vào Firebase |
| `htvc/heartbeat/+` | Subscribe | Cập nhật `lastSeen` |

---

## 🔌 Topic Subscription Map (ESP32)

| Topic | Subscription | Xử lý |
|-------|--------------|--------|
| `htvc/control/floor1` | Subscribe | Bật GPIO, gửi ACK |
| `htvc/sensors/floor1` | Publish | Gửi dữ liệu cảm biến |
| `htvc/control/ack/floor1` | Publish | Gửi xác nhận lệnh |
| `htvc/heartbeat/floor1` | Publish | Gửi nhịp tim |
| `htvc/status/floor1` | Publish | Gửi trạng thái on-demand |

---

## 🧪 Test Topics bằng MQTT Explorer

### 1. Test xem ESP32 gửi dữ liệu:
```
Subscribe: htvc/sensors/floor1
```
→ Mỗi 5 giây sẽ thấy JSON mới

### 2. Test gửi lệnh tắt LED:
```
Publish to: htvc/control/floor1

Payload:
{
  "target": "led",
  "value": false,
  "commandId": "test_001"
}
```
→ Nếu ESP32 được powered, LED sẽ tắt + ACK sẽ gửi ra `htvc/control/ack/floor1`

### 3. Test xem ESP32 còn online không:
```
Subscribe: htvc/heartbeat/floor1
```
→ Nếu không thấy message trong 30 giây → ESP32 offline

### 4. Test lấy trạng thái hiện tại:
```
Subscribe: htvc/status/floor1
```
→ Sẽ nhận được JSON trạng thái tức thời

---

## 🔐 Bảo mật & Best Practices

1. **QoS Level:**
   - Cảm biến & Heartbeat: QoS 1 (đủ, không cần đảm bảo tuyệt đối)
   - Điều khiển: QoS 1 (phải đảm bảo lệnh đến)

2. **Credential Management:**
   - Không commit `MQTT_PASSWORD` vào git
   - Dùng `.env` file để lưu:
     ```
     MQTT_BROKER=...
     MQTT_USER=HTVC_WEB
     MQTT_PASSWORD=Turauthongminh1
     MQTT_PORT=8883
     ```

3. **Topic Naming Convention:**
   - Prefix: `htvc/` (dùng chung cho tất cả devices)
   - Sub-prefix: `sensors/`, `control/`, `heartbeat/`
   - Device ID: `floor1`, `floor2`, `floor3`
   - Ví dụ: `htvc/sensors/floor1`

4. **Retain Messages:**
   - `htvc/status/floor1` → Retain = True (lưu trạng thái cuối)
   - `htvc/control/ack/floor1` → Retain = False (ACK không cần lưu)

---

## 📝 Ví dụ Workflow Hoàn Chỉnh: Bạn nhấn nút Bơm trên Web

```
1. Web Dashboard
   ↓ (Click "Bắt bơm")
   
2. JavaScript gọi: POST /api/dashboard/control
   {
     "deviceId": "htvc-floor1-001",
     "floor": 1,
     "target": "pump",
     "value": true
   }
   
3. Flask Server
   ↓ (Lưu vào Firebase + Publish)
   
4. HiveMQ Cloud
   Publish → htvc/control/floor1
   {
     "target": "pump",
     "value": true,
     "commandId": "cmd_xxx"
   }
   
5. ESP32 nhận từ htvc/control/floor1
   ↓ (Bật GPIO Pump)
   digitalWrite(PUMP_PIN_1, HIGH)
   
6. ESP32 publish → htvc/control/ack/floor1
   {
     "deviceId": "htvc-floor1-001",
     "target": "pump",
     "value": true,
     "success": true,
     "commandId": "cmd_xxx"
   }
   
7. Flask Server nhận ACK
   ↓ (Log success, cập nhật Firebase)
   
8. Web Dashboard cập nhật
   ↓ (Nút "Bơm" chuyển sang "ON", Toast báo thành công)
   
9. Sau 5 giây, ESP32 publish → htvc/sensors/floor1
   {
     ...
     "data": {
       "pump": 1,  ← Trạng thái thực tế
       ...
     }
   }
   
10. Firebase & Web cập nhật trạng thái từ dữ liệu cảm biến
```

---

## 🛠️ Debug & Troubleshooting

### ESP32 không nhận lệnh?
1. Check: ESP32 có subscribe `htvc/control/floor1` không?
   ```cpp
   // Trong main.cpp, kiểm tra onControlMessage() được gọi
   ```
2. Check: HiveMQ Console xem có message gửi vào `htvc/control/floor1` không?
3. Check: ESP32 Serial Monitor xem có log `[MQTT] Nhan lenh tu Web:`?

### Web không cập nhật trạng thái?
1. Check: Firebase có nhận `Readings` document không?
2. Check: Flask Server có running không? `python app.py`
3. Check: MQTT Listener có start không? Xem log `[MQTT] Listener started`

### Heartbeat không gửi?
1. Check ESP32 cứ mỗi 30 giây publish lên `htvc/heartbeat/floor1`
2. Nếu không thấy → ESP32 mất kết nối MQTT
3. Kiểm tra WiFi signal strength (RSSI)

---

## 📖 Tham khảo Thêm

- **MQTT Client Tools:** MQTT Explorer, HiveMQ Web UI
- **HiveMQ Dashboard:** https://console.hivemq.cloud
- **Firebase Firestore:** https://console.firebase.google.com
- **ArduinoJson:** https://arduinojson.org (Dùng trong ESP32)
- **PubSubClient:** https://github.com/knolleary/pubsubclient (MQTT library)

---

**Ngày cập nhật:** 28/04/2026  
**Phiên bản:** 1.0
