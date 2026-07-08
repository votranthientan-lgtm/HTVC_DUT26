"""
blueprints/api/routes.py - API endpoints for HTVC
Handles sensor readings, control commands, and device management.
"""

import logging
from numbers import Number
from flask import Blueprint, jsonify, request
from firebase.htvc_data import HTVCData
from firebase.init import get_db

log = logging.getLogger("htvc.api")
bp = Blueprint("api", __name__, url_prefix="/api")

# Default device ID
DEFAULT_DEVICE_ID = "htvc-floor1-001"
ALLOWED_CONTROL_TARGETS = {"led", "pump", "fan"}


# ═══════════════════════════════════════════════════════════════════════════════
# READINGS API
# ═══════════════════════════════════════════════════════════════════════════════

@bp.route("/readings", methods=["POST"])
def receive_readings():
    """
    Receive sensor readings from ESP32.
    ESP32 sends POST request with JSON data.

    Expected JSON format:
    {
        "deviceId": "htvc-floor1-001",
        "floors": {
            "1": {"tempAir": 25.5, "humidity": 60, "tds": 450, "pump": 0, "led": 0},
            "2": {...},
            "3": {...}
        }
    }
    """
    try:
        data = request.get_json()
        if not data:
            return jsonify({"error": "No data provided"}), 400

        device_id = data.get("deviceId", DEFAULT_DEVICE_ID)
        device_name = data.get("deviceName", f"Tủ Rau - {device_id}")
        floors = data.get("floors", {})

        # Process each floor's data
        for floor_num, floor_data in floors.items():
            try:
                floor = int(floor_num)
                HTVCData.save_reading(
                    device_id=device_id,
                    device_name=device_name,
                    floor=floor,
                    data=floor_data
                )
            except Exception as e:
                log.error(f"Error saving floor {floor_num} data: {e}")

        # Check for pending commands and return them
        commands = []
        for floor_num in floors.keys():
            try:
                floor = int(floor_num)
                pending = HTVCData.get_pending_commands(device_id, floor)
                commands.extend(pending)
            except Exception as e:
                log.error(f"Error getting commands for floor {floor_num}: {e}")

        response = {"status": "ok", "commands": commands}
        return jsonify(response), 200

    except Exception as e:
        log.error(f"Error receiving readings: {e}")
        return jsonify({"error": str(e)}), 500


@bp.route("/readings/latest", methods=["GET"])
def get_latest_reading():
    """
    Get the latest reading for a device.

    Query params:
    - deviceId: Device identifier (optional, defaults to DEFAULT_DEVICE_ID)
    - floor: Floor number (optional)

    Returns latest sensor reading from Firebase.
    """
    device_id = request.args.get("deviceId", DEFAULT_DEVICE_ID)
    floor = request.args.get("floor", type=int)

    try:
        # If floor specified, get history and return latest
        if floor:
            history = HTVCData.get_reading_history(device_id, floor=floor, limit=1)
            if history:
                return jsonify(history[0]), 200
        else:
            reading = HTVCData.get_latest_reading(device_id)
            if reading:
                return jsonify(reading), 200

        return jsonify({"error": "No readings found"}), 404

    except Exception as e:
        log.error(f"Error getting latest reading: {e}")
        return jsonify({"error": str(e)}), 500


@bp.route("/readings/history", methods=["GET"])
def get_reading_history():
    """
    Get historical readings for a device.

    Query params:
    - deviceId: Device identifier
    - floor: Floor number (optional)
    - limit: Maximum number of readings (default 100)

    Returns list of historical readings from Firebase.
    """
    device_id = request.args.get("deviceId", DEFAULT_DEVICE_ID)
    floor = request.args.get("floor", type=int)
    limit = request.args.get("limit", 100, type=int)

    try:
        history = HTVCData.get_reading_history(device_id, floor=floor, limit=limit)
        return jsonify(history), 200
    except Exception as e:
        log.error(f"Error getting reading history: {e}")
        return jsonify({"error": str(e)}), 500


# ═══════════════════════════════════════════════════════════════════════════════
# CONTROL API
# ═══════════════════════════════════════════════════════════════════════════════
@bp.route("/control", methods=["POST"])
def send_control():
    try:
        data = request.get_json()
        if not data:
            return jsonify({"error": "No data provided"}), 400

        device_id = data.get("deviceId", DEFAULT_DEVICE_ID)
        floor = data.get("floor", 1)
        target = data.get("target")
        value = data.get("value")
        try:
            floor = int(floor)
        except (TypeError, ValueError):
            return jsonify({"error": "Invalid floor"}), 400
        if floor not in (1, 2, 3):
            return jsonify({"error": "Invalid floor"}), 400
        if not target:
            return jsonify({"error": "Missing target"}), 400
        target = str(target).strip().lower()
        if target not in ALLOWED_CONTROL_TARGETS:
            return jsonify({"error": "Invalid target"}), 400
        if not isinstance(value, (bool, Number)):
            return jsonify({"error": "Invalid value"}), 400
        if isinstance(value, Number) and not isinstance(value, bool):
            value = bool(value)

        log.info(
            "[WEB->API] control request ip=%s deviceId=%s floor=%s target=%s value=%s",
            request.remote_addr,
            device_id,
            floor,
            target,
            value,
        )

        # Lưu vào Firebase trước
        command_id = HTVCData.save_control_command(
            device_id=device_id, floor=floor, target=target, value=value, source="dashboard"
        )

        # Gửi qua MQTT (Đã ép kiểu an toàn)
        try:
            from mqtt.listener import send_control_command
            # Ép kiểu để không bị lỗi Object of type Sentinel
            safe_device_id = str(device_id)
            safe_floor = floor
            safe_target = target
            safe_value = value

            mqtt_ok = send_control_command(safe_device_id, safe_floor, safe_target, safe_value, command_id)
            log.info(
                "[API->MQTT] publish result=%s topic=htvc/control/floor%s target=%s",
                mqtt_ok,
                safe_floor,
                safe_target,
            )
        except Exception as mqtt_error:
            log.warning(f"MQTT publish failed: {mqtt_error}")

        return jsonify({
            "status": "ok",
            "commandId": command_id,
            "mqttPublished": bool(locals().get("mqtt_ok", False)),
        }), 200
    except Exception as e:
        return jsonify({"error": str(e)}), 500


@bp.route("/control/latest", methods=["GET"])
def get_latest_control():
    """
    Get the latest pending control commands for a device.

    Query params:
    - deviceId: Device identifier
    - floor: Floor number

    ESP32 calls this to check for pending commands.
    """
    device_id = request.args.get("deviceId", DEFAULT_DEVICE_ID)
    floor = request.args.get("floor", 1, type=int)

    try:
        commands = HTVCData.get_pending_commands(device_id, floor)

        # Format response for ESP32
        control = {}
        for cmd in commands:
            target = cmd.get("target")
            value = cmd.get("value")
            if target:
                control[target] = value
            # Mark as executed
            HTVCData.mark_command_executed(cmd.get("id", ""))

        return jsonify(control), 200

    except Exception as e:
        log.error(f"Error getting control commands: {e}")
        return jsonify({"error": str(e)}), 500


# ═══════════════════════════════════════════════════════════════════════════════
# HEARTBEAT API
# ═══════════════════════════════════════════════════════════════════════════════

@bp.route("/heartbeat", methods=["POST"])
def receive_heartbeat():
    """
    Receive heartbeat/status from ESP32 device.

    Expected JSON format:
    {
        "deviceId": "htvc-floor1-001",
        "ip": "192.168.1.100",
        "rssi": -45,
        "free_heap": 123456,
        "uptime": 3600,
        "status": "online"
    }
    """
    try:
        data = request.get_json()
        if not data:
            return jsonify({"error": "No data provided"}), 400

        device_id = data.get("deviceId", DEFAULT_DEVICE_ID)

        HTVCData.save_device_heartbeat(
            device_id=device_id,
            ip_address=data.get("ip", ""),
            rssi=data.get("rssi", 0),
            free_heap=data.get("free_heap", 0),
            uptime=data.get("uptime", 0)
        )

        return jsonify({"status": "ok"}), 200

    except Exception as e:
        log.error(f"Error receiving heartbeat: {e}")
        return jsonify({"error": str(e)}), 500


# ═══════════════════════════════════════════════════════════════════════════════
# DEVICE API
# ═══════════════════════════════════════════════════════════════════════════════

@bp.route("/devices", methods=["GET"])
def get_devices():
    """
    Get all registered devices.

    Returns list of all devices with their latest status.
    """
    try:
        devices = HTVCData.get_all_devices()
        return jsonify(devices), 200
    except Exception as e:
        log.error(f"Error getting devices: {e}")
        return jsonify({"error": str(e)}), 500


@bp.route("/devices/<device_id>", methods=["GET"])
def get_device(device_id):
    """
    Get details for a specific device.

    Returns device information and latest reading.
    """
    try:
        reading = HTVCData.get_latest_reading(device_id)
        if reading:
            return jsonify(reading), 200
        return jsonify({"error": "Device not found"}), 404
    except Exception as e:
        log.error(f"Error getting device {device_id}: {e}")
        return jsonify({"error": str(e)}), 500


# ═══════════════════════════════════════════════════════════════════════════════
# STATS API
# ═══════════════════════════════════════════════════════════════════════════════

@bp.route("/stats", methods=["GET"])
def get_stats():
    """
    Get dashboard statistics.

    Returns aggregated statistics for the dashboard.
    """
    try:
        db = get_db()

        # Count readings
        readings = list(db.collection("Readings").stream())
        total_readings = len(readings)

        # Count devices
        devices = list(db.collection("Devices").stream())
        online_devices = sum(
            1 for d in devices
            if d.to_dict().get("status") == "online"
        )

        # Get latest values
        latest = {}
        if readings:
            for doc in readings[:10]:
                data = doc.to_dict()
                floor = data.get("floor", 1)
                if floor not in latest:
                    latest[floor] = {
                        "tempAir": data.get("tempAir", 0),
                        "humidity": data.get("humidity", 0),
                        "tds": data.get("tds", 0),
                    }

        return jsonify({
            "totalReadings": total_readings,
            "totalDevices": len(devices),
            "onlineDevices": online_devices,
            "latestValues": latest,
        }), 200

    except Exception as e:
        log.error(f"Error getting stats: {e}")
        return jsonify({"error": str(e)}), 500


# ═══════════════════════════════════════════════════════════════════════════════
# WEBSOCKET SUPPORT (via polling fallback)
# ═══════════════════════════════════════════════════════════════════════════════

@bp.route("/ws/events", methods=["GET"])
def ws_events():
    """
    Fallback endpoint for WebSocket-like real-time updates.
    Clients poll this endpoint to get updates.

    Returns recent events/updates since last poll.
    """
    try:
        device_id = request.args.get("deviceId", DEFAULT_DEVICE_ID)

        # Get recent readings
        recent = HTVCData.get_reading_history(device_id, limit=5)

        events = []
        for reading in recent:
            events.append({
                "type": "update",
                "data": reading,
                "timestamp": reading.get("timestamp"),
            })

        return jsonify({"events": events}), 200

    except Exception as e:
        log.error(f"Error getting events: {e}")
        return jsonify({"error": str(e)}), 500
