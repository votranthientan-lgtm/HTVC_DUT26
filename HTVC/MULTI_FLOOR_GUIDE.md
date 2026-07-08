# Multi-Floor (3 Tầng) - Hướng Dẫn Tích Hợp

## Tổng Quan
ESP32 hiện hỗ trợ nhận và xử lý dữ liệu từ **3 tầng** (T1, T2, T3) thông qua UART từ Arduino Mega.

---

## 1. Format Mã Hóa Dữ Liệu từ Mega → ESP32

### **Option A: Format Đơn (DATA) - Tầng 1 Duy Nhất**
Nếu chỉ gửi dữ liệu 1 tầng (tương thích với cũ):
```
DATA:{"time":"2026-05-05 10:30:45","tds":1250,"tempA":28.5,"hum":65.3,"ph":7.0,"tempW":25.2,"pTuoi":0,"pAB":0,"pCD":0,"led":1}
```

ESP32 sẽ lưu vào `_floors[0]` (T1) và publish lên `htvc/sensors/floor1`

---

### **Option B: Format Multi-Floor (DATA_MULTI) - 3 Tầng**
Để gửi dữ liệu cho cả 3 tầng trong 1 message:

```
DATA_MULTI:{"t1":{"time":"2026-05-05 10:30:45","tds":1250,"tempA":28.5,"hum":65.3,"ph":7.0,"tempW":25.2,"pTuoi":0,"pAB":0,"pCD":0,"led":1},"t2":{"time":"2026-05-05 10:30:45","tds":1300,"tempA":29.0,"hum":70.0,"ph":6.8,"tempW":26.0,"pTuoi":1,"pAB":0,"pCD":0,"led":0},"t3":{"time":"2026-05-05 10:30:45","tds":1200,"tempA":27.5,"hum":60.0,"ph":7.2,"tempW":24.0,"pTuoi":0,"pAB":1,"pCD":0,"led":0}}
```

**Cấu trúc JSON:**
```json
DATA_MULTI:{
  "t1": {
    "time": "2026-05-05 10:30:45",
    "tds": 1250,
    "tempA": 28.5,
    "hum": 65.3,
    "ph": 7.0,
    "tempW": 25.2,
    "pTuoi": 0,      // Pump Tưới (main pump)
    "pAB": 0,        // Pump AB
    "pCD": 0,        // Pump CD
    "led": 1         // LED On/Off
  },
  "t2": { ... },
  "t3": { ... }
}
```

---

## 2. Cấu Hình Tầng & MQTT Topics

| Tầng | Biến Trong JSON | MQTT Topic Nhận | Device ID | Firestore Doc |
|------|-----------------|-----------------|-----------|----------------|
| 1    | `t1`           | `htvc/sensors/floor1` | `htvc-floor1-001` | readings-floor1 |
| 2    | `t2`           | `htvc/sensors/floor2` | `htvc-floor2-001` | readings-floor2 |
| 3    | `t3`           | `htvc/sensors/floor3` | `htvc-floor3-001` | readings-floor3 |

---

## 3. Flow Dữ Liệu

```
Mega (3 Cảm Biến)
    ↓ Serial2 (115200 baud, UART)
    ↓ DATA_MULTI:{...}
    ↓
ESP32 (esp_data.cpp)
    ├─ parseLine() → phát hiện "DATA_MULTI:"
    ├─ parseMultiFloor() → parse JSON, lưu vào _floors[0..2]
    ├─ render() → in tất cả 3 tầng ra Serial Monitor
    └─ applyOutputs() → điều khiển GPIO dựa trên T1
        ↓ (main.cpp → publishTelemetry)
        ↓
MQTT Broker (HiveMQ Cloud)
    ├─ htvc/sensors/floor1 (T1 data)
    ├─ htvc/sensors/floor2 (T2 data)
    └─ htvc/sensors/floor3 (T3 data)
        ↓ (mqtt/listener.py)
        ↓
Firestore
    ├─ Readings/readings-floor1 (lưu T1)
    ├─ Readings/readings-floor2 (lưu T2)
    └─ Readings/readings-floor3 (lưu T3)
        ↓ (blueprints/dashboard/routes.py)
        ↓
Web Dashboard
    ├─ [Floor 1] TDS: 1250 ppm, Temp: 28.5°C, pH: 7.0, Pump: OFF
    ├─ [Floor 2] TDS: 1300 ppm, Temp: 29.0°C, pH: 6.8, Pump: ON
    └─ [Floor 3] TDS: 1200 ppm, Temp: 27.5°C, pH: 7.2, Pump: ON
```

---

## 4. Hàm Mã Hóa (C/Arduino) - Mẫu Cho Mega

```cpp
#include <ArduinoJson.h>

void sendMultiFloorData() {
  JsonDocument doc;
  
  // Floor 1
  doc["t1"]["time"] = getTime();  // "2026-05-05 10:30:45"
  doc["t1"]["tds"] = tds1;        // 1250
  doc["t1"]["tempA"] = tempA1;    // 28.5
  doc["t1"]["hum"] = humidity1;   // 65.3
  doc["t1"]["ph"] = pH1;          // 7.0
  doc["t1"]["tempW"] = tempW1;    // 25.2
  doc["t1"]["pTuoi"] = pumpStatus1; // 0
  doc["t1"]["pAB"] = pumpAB1;     // 0
  doc["t1"]["pCD"] = pumpCD1;     // 0
  doc["t1"]["led"] = ledStatus1;  // 1
  
  // Floor 2 (tương tự)
  doc["t2"]["time"] = getTime();
  doc["t2"]["tds"] = tds2;
  // ... remaining fields
  
  // Floor 3 (tương tự)
  // ...
  
  // Gửi qua Serial2
  Serial2.print("DATA_MULTI:");
  serializeJson(doc, Serial2);
  Serial2.println();  // QUAN TRỌNG: kết thúc bằng newline (\n)
}
```

---

## 5. Cấu Hình UART trên Mega

**Pinout:**
- Mega TX → ESP32 RX (GPIO 16)
- Mega RX → ESP32 TX (GPIO 17)
- Mega GND → ESP32 GND

**Code khởi tạo:**
```cpp
void setup() {
  Serial.begin(115200);      // Serial Monitor debug
  Serial2.begin(115200);     // Kết nối tới ESP32
  Serial.println("Mega started, sending data to ESP32...");
}

void loop() {
  // Đọc cảm biến
  readAllSensors();
  
  // Gửi data
  sendMultiFloorData();
  
  delay(5000);  // Gửi mỗi 5 giây
}
```

---

## 6. Debugging trên ESP32

Mở **Serial Monitor** tại baud rate **115200**, bạn sẽ thấy:

### **Khi nhận dữ liệu thành công:**
```
====== MONITORING (FROM MEGA) ======
--- FLOOR 1 ---
Time: 2026-05-05 10:30:45
Air : 28.5C | 65.3%
Water: 25.2C | pH: 7.00
TDS : 1250 ppm
Pump: OFF | Pump AB: OFF | Pump CD: OFF | LED: ON

--- FLOOR 2 ---
Time: 2026-05-05 10:30:45
Air : 29.0C | 70.0%
Water: 26.0C | pH: 6.80
TDS : 1300 ppm
Pump: ON | Pump AB: OFF | Pump CD: OFF | LED: OFF

--- FLOOR 3 ---
Time: 2026-05-05 10:30:45
Air : 27.5C | 60.0%
Water: 24.0C | pH: 7.20
TDS : 1200 ppm
Pump: OFF | Pump AB: ON | Pump CD: OFF | LED: OFF
====================================
```

### **Khi JSON sai format:**
```
UART Multi-Floor Error: [error message]
```

---

## 7. API Changes - Lấy Data Cho Tầng Cụ Thể

```cpp
// Lấy dữ liệu cho tầng cụ thể
const FloorData& floor1Data = sensorData.getFloorData(1);
const FloorData& floor2Data = sensorData.getFloorData(2);
const FloorData& floor3Data = sensorData.getFloorData(3);

// Hoặc sử dụng trực tiếp (tương thích cũ, lấy T1)
float tds = sensorData.getTds();           // Lấy T1
float temp = sensorData.getTempAir();      // Lấy T1
int led = sensorData.getLED();             // Lấy T1

// Build telemetry cho tầng cụ thể
JsonDocument doc;
sensorData.buildTelemetry(doc, 2);  // Lấy data T2
sensorData.buildTelemetry(doc, 3);  // Lấy data T3
```

---

## 8. Kiểm Tra Kết Nối

✅ **Nếu dữ liệu có thể thấy:**
1. Mở Serial Monitor (ESP32)
2. Kiểm tra `====== MONITORING (FROM MEGA) ======` xuất hiện
3. Dữ liệu 3 tầng được in
4. Mở Firestore Console, kiểm tra `Readings` collection
   - `readings-floor1`, `readings-floor2`, `readings-floor3` được tạo
5. Mở Web Dashboard, kiểm tra dữ liệu cảm biến từ 3 tầng

❌ **Nếu không thấy dữ liệu:**
1. Kiểm tra kết nối UART: Mega TX → ESP32 RX (GPIO 16)
2. Kiểm tra baud rate: cả 2 phải là **115200**
3. Kiểm tra format JSON: prefix phải là `DATA:` hoặc `DATA_MULTI:`
4. Kiểm tra Mega có thực sự gửi dữ liệu (dùng một Serial Monitor khác cho Mega)

---

## 9. Backward Compatibility

**Hệ thống vẫn hỗ trợ format cũ (DATA):**
- Nếu Mega gửi: `DATA:{...}` (1 tầng) → lưu vào floor 1
- Nếu Mega gửi: `DATA_MULTI:{...}` (3 tầng) → lưu vào cả 3 floors

**Không cần sửa gì cả nếu chỉ dùng 1 tầng!**

---

## 10. Bước Tiếp Theo

1. **Cập nhật Mega code** để gửi `DATA_MULTI` format
2. **Upload firmware mới** lên ESP32
3. **Monitor Serial output** để xác nhận data nhận đúng
4. **Check Firestore** có 3 document readings
5. **Test Web Dashboard** hiển thị dữ liệu từ 3 tầng

---

## Tệp Liên Quan
- `esp_data.h` - Cấu trúc `FloorData`, khai báo hàm
- `esp_data.cpp` - Triển khai `parseMultiFloor()`, `render()`, `buildTelemetry()`
- `main.cpp` - Publish telemetry cho 3 tầng
- `htvc_config.h` - MQTT topics cho 3 tầng (TOPIC_SENSORS_T1/T2/T3)
