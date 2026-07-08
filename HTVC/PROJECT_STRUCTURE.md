# HTVC - Tủ Rau Thông Minh Project

## Cấu trúc dự án

```
HTVC/
├── app.py                      # Flask application entry point
├── requirements.txt            # Python dependencies
├── README.md                   # Documentation
├── .env.example               # Environment variables template
│
├── esp32/                     # ESP32 firmware
│   └── htvc/
│       ├── platformio.ini     # PlatformIO configuration
│       ├── include/
│       │   └── esp_data.h     # Header file
│       └── src/
│           ├── main.cpp       # Main firmware code
│           └── esp_data.cpp   # Sensor data handler
│
├── firebase/                  # Firebase integration
│   ├── init.py               # Firebase initialization
│   └── htvc_data.py         # Data operations
│
├── mqtt/                      # MQTT listener
│   ├── __init__.py
│   └── listener.py           # HiveMQ listener
│
├── blueprints/               # Flask blueprints
│   ├── api/
│   │   ├── __init__.py
│   │   └── routes.py         # API endpoints
│   ├── dashboard/
│   │   ├── __init__.py
│   │   └── routes.py         # Dashboard routes
│   └── student/
│       ├── __init__.py
│       └── routes.py         # Student portal
│
├── templates/                # HTML templates
│   ├── base.html            # Base template
│   ├── dashboard.html       # Main dashboard
│   ├── controls.html        # Device controls
│   ├── floors.html          # Floor details
│   ├── history.html         # Historical data
│   ├── settings.html        # Settings page
│   └── portal.html          # Public portal
│
└── static/                 # Static files
    ├── css/
    │   └── dashboard.css    # Dashboard styles
    ├── js/
    │   └── dashboard.js     # Dashboard JavaScript
    ├── images/
    ├── icons/
    └── fonts/
```

## Thiết lập nhanh

### 1. Python Backend

```bash
cd HTVC
python -m venv venv
source venv/bin/activate  # Windows: venv\Scripts\activate
pip install -r requirements.txt
python app.py
```

### 2. Firebase

1. Tạo project Firebase tại https://console.firebase.google.com
2. Enable Firestore Database
3. Tải service account key (JSON)
4. Đặt file JSON vào folder HTVC hoặc set biến môi trường

### 3. ESP32 Firmware

```bash
cd esp32/htvc
pio run -t upload
```

Cấu hình WiFi và server URL trong `src/main.cpp`

## MQTT Topics

| Topic | Direction | Mô tả |
|-------|-----------|--------|
| `htvc/sensors/floor1` | ESP32 → Server | Dữ liệu cảm biến |
| `htvc/control/floor1` | Server → ESP32 | Lệnh điều khiển |
| `htvc/status/floor1` | ESP32 → Server | Trạng thái thiết bị |
| `htvc/heartbeat/floor1` | ESP32 → Server | Heartbeat |

## API Endpoints

### Readings
- `POST /api/readings` - Nhận dữ liệu từ ESP32
- `GET /api/readings/latest` - Đọc mới nhất
- `GET /api/readings/history` - Lịch sử dữ liệu

### Control
- `POST /api/control` - Gửi lệnh điều khiển
- `GET /api/control/latest` - Lấy lệnh chờ

### Dashboard
- `GET /dashboard` - Trang dashboard
- `GET /api/dashboard/stats` - Thống kê
- `POST /api/dashboard/control` - Điều khiển

## ESP32 Data Format

```json
{
  "deviceId": "htvc-floor1-001",
  "floors": {
    "1": {
      "tempAir": 25.5,
      "humidity": 60,
      "tds": 450,
      "ph": 6.5,
      "tempWater": 22.0,
      "pump": 0,
      "led": 1
    }
  }
}
```

## Control Command Format

```json
{
  "deviceId": "htvc-floor1-001",
  "floor": 1,
  "target": "led",
  "value": true
}
```
