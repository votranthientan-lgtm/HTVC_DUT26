# HTVC - Tủ Rau Thông Minh Backend

Flask backend for HTVC Smart Vegetable Rack System

## Requirements

```
Flask>=2.3.0
flask-cors>=4.0.0
firebase-admin>=6.0.0
paho-mqtt>=1.6.0
python-dotenv>=1.0.0
gunicorn>=21.0.0
```

## Installation

1. Create virtual environment:
```bash
python -m venv venv
source venv/bin/activate  # Linux/Mac
venv\Scripts\activate     # Windows
```

2. Install dependencies:
```bash
pip install -r requirements.txt
```

3. Set up environment variables:
```bash
# Firebase
export FIREBASE_SERVICE_ACCOUNT="path/to/serviceAccountKey.json"
# Or use JSON string
export FIREBASE_CREDENTIALS_JSON='{"type": "service_account", ...}'

# MQTT (optional)
export MQTT_BROKER="broker.hivemq.com"
export MQTT_PORT="1883"
```

4. Run the application:
```bash
python app.py
```

The application will start at http://localhost:5000

## API Endpoints

### Readings
- `POST /api/readings` - Receive sensor data from ESP32
- `GET /api/readings/latest` - Get latest reading
- `GET /api/readings/history` - Get reading history

### Control
- `POST /api/control` - Send control command
- `GET /api/control/latest` - Get pending commands

### Device
- `GET /api/devices` - List all devices
- `GET /api/devices/<id>` - Get device details

### Dashboard
- `GET /dashboard` - Main dashboard page
- `GET /api/dashboard/stats` - Dashboard statistics
- `GET /api/dashboard/floor/<n>` - Floor data
- `POST /api/dashboard/control` - Dashboard control

## ESP32 Configuration

Update these in your ESP32 code:
```cpp
const char* WIFI_SSID = "YourWiFi";
const char* WIFI_PASSWORD = "YourPassword";
const char* serverUrl = "http://your-server.com:5000";
const char* DEVICE_ID = "htvc-floor1-001";
```

## MQTT Topics

- `htvc/sensors/floor1` - Sensor data
- `htvc/control/floor1` - Control commands
- `htvc/status/floor1` - Device status
- `htvc/heartbeat/floor1` - Heartbeat

## Firebase Collections

- `Devices` - Device registry
- `Readings` - Sensor readings history
- `Control` - Control commands
- `Logs` - System logs
