# ❌ Tại Sao Không Thấy Dữ Liệu Trên Web Dashboard

## 📌 Root Cause (Nguyên Nhân Gốc)

**Mega chưa gửi dữ liệu cho ESP32 qua UART Serial2**

Nếu Mega không gửi, toàn bộ pipeline bị dừng:

```
Mega (không gửi dữ liệu)
    ↓
ESP32 (không có dữ liệu)
    ↓
MQTT (không publish)
    ↓
Firestore (không update)
    ↓
Web (hiển thị "--, --, --, --")
```

---

## 🔍 Xác Minh Data Flow

### Bước 1: Kiểm Tra Mega → ESP32 (UART Serial2)

**Trên ESP32 Serial Monitor, bạn sẽ thấy:**
```
====== MONITORING (FROM MEGA) ======
--- FLOOR 1 ---
Time: 2026-05-05 19:30:45
Air : 28.5C | 65.3%
Water: 25.2C | pH: 7.00
TDS : 1250 ppm
...
====================================
```

**Nếu KHÔNG thấy output trên:**
```
❌ Mega không gửi dữ liệu
❌ Kết nối UART (Serial2) không hoạt động
❌ JSON format sai
```

---

### Bước 2: Kiểm Tra ESP32 → MQTT

**Nếu Mega gửi dữ liệu, ESP32 sẽ:**
1. Parse dữ liệu từ Serial2
2. Publish lên `htvc/sensors/floor1`, `htvc/sensors/floor2`, `htvc/sensors/floor3`
3. Mỗi 5 giây (SENSOR_PUBLISH_INTERVAL_MS)

Kiểm tra trong main.cpp:
```cpp
if (millis() - lastSensorUpdate >= htvc_config::SENSOR_PUBLISH_INTERVAL_MS) {
    // publishTelemetrySnapshot() được gọi
    // Dữ liệu được đẩy lên MQTT
}
```

---

### Bước 3: Kiểm Tra MQTT → Firestore

**Flask/MQTT Listener (mqtt/listener.py):**
- Subscribe tới `htvc/sensors/+` (tất cả floors)
- Khi nhận message, gọi `HTVCData.save_reading()`
- Lưu vào Firestore collection `Readings`

**Dấu hiệu thành công:**
```
[Sensors] htvc-floor1-001: Temp=28.5C, Hum=65.3%, TDS=1250.0
[Sensors] htvc-floor2-001: Temp=29.0C, Hum=70.0%, TDS=1300.0
[Sensors] htvc-floor3-001: Temp=27.5C, Hum=60.0%, TDS=1200.0
```

---

### Bước 4: Kiểm Tra Firestore → Web

**Dashboard quuy route:**
```
GET /api/dashboard/stats
    ↓ (blueprints/dashboard/routes.py)
    ↓ (HTVCData.get_latest_readings())
    ↓ (Firestore query)
    ↓ (Return JSON)
    ↓ (Web dashboard.js fetch)
    ↓ (Render sensor values)
```

---

## 🚀 Hướng Dẫn Fix

### **OPTION A: Cập Nhật Mega Code**

1. **Mega code phải gửi format này qua Serial2:**

```cpp
#include <ArduinoJson.h>

void sendData() {
  JsonDocument doc;
  
  // Floor 1
  doc["t1"]["time"] = "2026-05-05 19:30:45";
  doc["t1"]["tds"] = 1250;
  doc["t1"]["tempA"] = 28.5;
  doc["t1"]["hum"] = 65.3;
  doc["t1"]["ph"] = 7.0;
  doc["t1"]["tempW"] = 25.2;
  doc["t1"]["pTuoi"] = 0;
  doc["t1"]["pAB"] = 0;
  doc["t1"]["pCD"] = 0;
  doc["t1"]["led"] = 1;
  
  // Floor 2, 3... (tương tự)
  
  // Gửi qua Serial2
  Serial2.print("DATA_MULTI:");
  serializeJson(doc, Serial2);
  Serial2.println();
}
```

2. **Gọi hàm này mỗi 5 giây:**
```cpp
void loop() {
  static unsigned long lastSend = 0;
  
  if (millis() - lastSend >= 5000) {
    lastSend = millis();
    sendData();
  }
}
```

3. **Kiểm tra:**
   - Mega Serial Monitor → xem data được gửi
   - ESP32 Serial Monitor → xem "MONITORING (FROM MEGA)"
   - Web Dashboard → xem dữ liệu xuất hiện

---

### **OPTION B: Simulate Data Trên Web Server**

Tôi đã tạo `test_data_simulator.py` nhưng gặp HiveMQ authorization issue.

**Workaround đơn giản:** Tạo Flask endpoint test để insert test data trực tiếp:

```python
@app.route('/api/test/insert-sample-data', methods=['POST'])
def insert_sample_data():
    """Insert sample data directly to Firestore for testing"""
    from firebase.htvc_data import HTVCData
    
    test_data = {
        "tempA": 28.5, "hum": 65.3, "tds": 1250, "ph": 7.0, "tempW": 25.2
    }
    
    for floor in [1, 2, 3]:
        HTVCData.save_reading(
            device_id=f"htvc-floor{floor}-001",
            device_name=f"Tủ Rau - Tầng {floor}",
            floor=floor,
            data=test_data
        )
    
    return {"status": "ok", "message": "Test data inserted"}
```

---

## 📋 Checklist

- [ ] Mega code: Gửi `DATA_MULTI` format qua Serial2
- [ ] ESP32: Xác nhận nhận dữ liệu từ Serial2 (check Serial Monitor)
- [ ] ESP32: Publish lên MQTT (check logs)
- [ ] Flask: Nhận MQTT message (check console logs)
- [ ] Firestore: Có documents `readings-floor1/2/3` (check Firebase Console)
- [ ] Web Dashboard: Hiển thị dữ liệu 3 tầng (check http://localhost:5000)

---

## 🔗 Liên Quan

- [MULTI_FLOOR_GUIDE.md](../MULTI_FLOOR_GUIDE.md) - Hướng dẫn 3 tầng cho Mega
- `test_data_simulator.py` - Script test
- `mqtt/listener.py` - MQTT listener
- `firebase/htvc_data.py` - Firestore operations
- `blueprints/dashboard/routes.py` - API endpoint
