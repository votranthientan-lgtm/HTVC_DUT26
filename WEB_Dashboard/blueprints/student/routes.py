"""
blueprints/student/routes.py - Student portal for HTVC
Simple public-facing page for viewing vegetable rack status.
"""

from flask import Blueprint, jsonify, redirect, url_for

bp = Blueprint("student", __name__, url_prefix="/portal")


@bp.route("/")
def index():
    """Redirect portal path to the main floor monitoring page."""
    return redirect(url_for("dashboard.floors_page"))


@bp.route("/api/status")
def get_status():
    """
    Get current status of all floors.
    Public endpoint for quick status view.
    """
    try:
        from firebase.htvc_data import HTVCData

        floors = {}
        for floor in [1, 2, 3]:
            device_id = f"htvc-floor{floor}-001"
            reading = HTVCData.get_latest_reading(device_id)
            if reading:
                floors[floor] = {
                    "tempAir": reading.get("tempAir", 0),
                    "humidity": reading.get("humidity", 0),
                    "tds": reading.get("tds", 0),
                    "ph": reading.get("ph", 0),
                    "pump": reading.get("pump", False),
                    "led": reading.get("led", False),
                    "online": True
                }
            else:
                floors[floor] = {
                    "tempAir": 0,
                    "humidity": 0,
                    "tds": 0,
                    "ph": 0,
                    "pump": False,
                    "led": False,
                    "online": False
                }

        return jsonify({
            "status": "ok",
            "floors": floors
        }), 200

    except Exception as e:
        return jsonify({
            "status": "error",
            "message": str(e)
        }), 500
