"""
blueprints/dashboard/routes.py - Dashboard routes for HTVC
Serves the main dashboard web interface.
"""

import logging
import json
import queue
import time
from numbers import Number
from flask import Blueprint, Response, jsonify, render_template, request, stream_with_context, session
from firebase.htvc_data import HTVCData

log = logging.getLogger("htvc.dashboard")
bp = Blueprint("dashboard", __name__)
ALLOWED_CONTROL_TARGETS = {"led", "pump", "fan"}


@bp.route("/api/dashboard/stream")
def dashboard_stream():
    """Stream real-time MQTT updates to the browser using SSE."""
    from mqtt.listener import register_event_queue, unregister_event_queue

    event_queue = register_event_queue()

    def event_stream():
        try:
            yield "event: connected\ndata: {\"status\":\"ok\"}\n\n"
            while True:
                try:
                    event = event_queue.get(timeout=15)
                    yield f"data: {json.dumps(event)}\n\n"
                except queue.Empty:
                    yield ": keepalive\n\n"
        finally:
            unregister_event_queue(event_queue)

    return Response(
        stream_with_context(event_stream()),
        mimetype="text/event-stream",
        headers={
            "Cache-Control": "no-cache",
            "X-Accel-Buffering": "no",
            "Connection": "keep-alive",
        },
    )


@bp.route("/")
def index():
    """Main dashboard page."""
    return render_template("dashboard.html", active="dashboard")


@bp.route("/dashboard")
def dashboard():
    """Dashboard page."""
    return render_template("dashboard.html", active="dashboard")


@bp.route("/floors")
def floors_page():
    """Floor details page."""
    return render_template("floors.html", active="floors")


@bp.route("/floors/<int:floor_num>")
def floor_detail(floor_num):
    """Individual floor detail page."""
    if floor_num not in [1, 2, 3]:
        return "Invalid floor number", 400
    return render_template("floor_detail.html", floor=floor_num, active="floors")


@bp.route("/controls")
def controls_page():
    """Device control page."""
    return render_template("controls.html", active="controls")


@bp.route("/history")
def history_page():
    """Historical data page."""
    return render_template("history.html", active="history")


@bp.route("/settings")
def settings_page():
    """Settings page."""
    return render_template("settings.html", active="settings")


# ═══════════════════════════════════════════════════════════════════════════════
# Dashboard API
# ═══════════════════════════════════════════════════════════════════════════════

@bp.route("/api/dashboard/stats")
def get_dashboard_stats():
    """
    Get dashboard statistics.
    """
    try:
        devices = HTVCData.get_all_devices()

        stats = {
            "totalDevices": len(devices),
            "onlineDevices": sum(
                1 for d in devices
                if d.get("status") == "online"
            ),
            "floors": {}
        }

        # Get latest reading for each floor
        for floor in [1, 2, 3]:
            reading = HTVCData.get_latest_reading(f"htvc-floor{floor}-001")
            if reading:
                stats["floors"][floor] = {
                    "tempAir": reading.get("tempAir", 0),
                    "humidity": reading.get("humidity", 0),
                    "tds": reading.get("tds", 0),
                    "ph": reading.get("ph", 0),
                    "pump": reading.get("pump", False),
                    "fan": reading.get("fan", False),
                    "peri": reading.get("peri", False),
                    "led": reading.get("led", False),
                }
            else:
                stats["floors"][floor] = {
                    "tempAir": 0,
                    "humidity": 0,
                    "tds": 0,
                    "ph": 0,
                    "pump": False,
                    "fan": False,
                    "peri": False,
                    "led": False,
                }

        return jsonify(stats), 200

    except Exception as e:
        log.error(f"Error getting dashboard stats: {e}")
        return jsonify({"error": str(e)}), 500


@bp.route("/api/dashboard/floor/<int:floor_num>")
def get_floor_data(floor_num):
    """
    Get data for a specific floor.
    """
    try:
        device_id = f"htvc-floor{floor_num}-001"

        # Get latest reading
        reading = HTVCData.get_latest_reading(device_id)

        # Get history
        history = HTVCData.get_reading_history(device_id, floor=floor_num, limit=50)

        return jsonify({
            "latest": reading,
            "history": history
        }), 200

    except Exception as e:
        log.error(f"Error getting floor {floor_num} data: {e}")
        return jsonify({"error": str(e)}), 500


@bp.route("/api/dashboard/control", methods=["POST"])
def dashboard_control():
    """
    Send control command from dashboard.
    """
    try:
        data = request.get_json()

        if not data:
            return jsonify({"error": "No data provided"}), 400

        device_id = data.get("deviceId", "htvc-floor1-001")
        floor = data.get("floor", 1)
        target = data.get("target")
        value = data.get("value")

        if not target:
            return jsonify({"error": "Missing target"}), 400
        try:
            floor = int(floor)
        except (TypeError, ValueError):
            return jsonify({"error": "Invalid floor"}), 400
        if floor not in (1, 2, 3):
            return jsonify({"error": "Invalid floor"}), 400

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

        # Save command to Firebase
        command_id = HTVCData.save_control_command(
            device_id=device_id,
            floor=floor,
            target=target,
            value=value,
            source="dashboard"
        )

        # Try to send via MQTT
        try:
            from mqtt.listener import send_control_command
            mqtt_ok = send_control_command(device_id, floor, target, value, command_id)
            log.info(
                "[API->MQTT] publish result=%s topic=htvc/control/floor%s target=%s",
                mqtt_ok,
                floor,
                target,
            )
        except Exception as mqtt_error:
            log.warning(f"MQTT send failed: {mqtt_error}")

        return jsonify({
            "status": "ok",
            "commandId": command_id,
            "mqttPublished": bool(locals().get("mqtt_ok", False)),
        }), 200

    except Exception as e:
        log.error(f"Error sending control: {e}")
        return jsonify({"error": str(e)}), 500


@bp.route("/api/test/insert-sample-data", methods=["POST"])
def insert_sample_data():
    """
    INSERT TEST DATA - For development/testing only
    
    Inserts sample sensor data directly to Firestore.
    Use this to test the web dashboard without needing actual Mega/ESP32.
    
    Request body: {
        "floors": [1, 2, 3]  // optional, defaults to all
    }
    """
    try:
        # Sample data for 3 floors (using web/Firestore field names)
        test_data = [
            {
                "floor": 1,
                "deviceId": "htvc-floor1-001",
                "deviceName": "Tủ Rau Thông Minh - Tầng 1",
                "tempAir": 28.5,
                "humidity": 65.3,
                "tds": 1250,
                "ph": 7.0,
                "tempWater": 25.2,
                "pump": 0,
                "fan": 1,
                "peri": 0,
                "led": 1,
                # Alternative names (for compatibility)
                "tempA": 28.5,
                "hum": 65.3,
                "tempW": 25.2,
            },
            {
                "floor": 2,
                "deviceId": "htvc-floor2-001",
                "deviceName": "Tủ Rau Thông Minh - Tầng 2",
                "tempAir": 29.0,
                "humidity": 70.0,
                "tds": 1300,
                "ph": 6.8,
                "tempWater": 26.0,
                "pump": 1,
                "fan": 0,
                "peri": 1,
                "led": 0,
                # Alternative names
                "tempA": 29.0,
                "hum": 70.0,
                "tempW": 26.0,
            },
            {
                "floor": 3,
                "deviceId": "htvc-floor3-001",
                "deviceName": "Tủ Rau Thông Minh - Tầng 3",
                "tempAir": 27.5,
                "humidity": 60.0,
                "tds": 1200,
                "ph": 7.2,
                "tempWater": 24.0,
                "pump": 0,
                "fan": 1,
                "peri": 0,
                "led": 0,
                # Alternative names
                "tempA": 27.5,
                "hum": 60.0,
                "tempW": 24.0,
            },
        ]

        request_data = request.get_json() or {}
        target_floors = request_data.get("floors", [1, 2, 3])

        inserted = []
        for data in test_data:
            if data["floor"] in target_floors:
                HTVCData.save_reading(
                    device_id=data["deviceId"],
                    device_name=data["deviceName"],
                    floor=data["floor"],
                    data=data
                )
                inserted.append(data["floor"])
                log.info(f"[TEST] Inserted sample data for Floor {data['floor']}")

        return jsonify({
            "status": "ok",
            "message": f"Inserted test data for floors: {inserted}",
            "floors": inserted
        }), 200

    except Exception as e:
        log.error(f"Error inserting test data: {e}")
        return jsonify({"error": str(e)}), 500
