"""
firebase/htvc_data.py - Firebase data operations for HTVC project
Handles sensor readings, device status, and control commands.
"""

from datetime import datetime
from typing import Optional, Dict, List, Any

try:
    from firebase_admin import firestore
    from .init import get_db
    _FIREBASE_AVAILABLE = True
except ModuleNotFoundError:
    firestore = None
    get_db = None
    _FIREBASE_AVAILABLE = False


class HTVCData:
    """Class to manage HTVC data in Firebase Firestore."""

    COLLECTION_DEVICES = "Devices"
    COLLECTION_READINGS = "Readings"
    COLLECTION_CONTROL = "Control"
    COLLECTION_LOGS = "Logs"

    @staticmethod
    def _firebase_unavailable(default_value=None):
        return default_value

    @staticmethod
    def save_reading(
        device_id: str,
        device_name: str,
        floor: int,
        data: Dict[str, Any]
    ) -> str:
        """
        Save a sensor reading to Firestore.

        Args:
            device_id: Unique device identifier
            device_name: Human-readable device name
            floor: Floor number (1, 2, or 3)
            data: Dictionary containing sensor readings

        Returns:
            Document ID of the created reading
        """
        if not _FIREBASE_AVAILABLE:
            return "firebase-disabled"

        db = get_db()

        reading_data = {
            "deviceId": device_id,
            "deviceName": device_name,
            "floor": floor,
            "timestamp": firestore.SERVER_TIMESTAMP,
            "createdAt": datetime.utcnow().isoformat(),

            # Sensor data
            "tempAir": data.get("tempAir", data.get("tempA", 0)),
            "humidity": data.get("humidity", data.get("hum", 0)),
            "tds": data.get("tds", 0),
            "ph": data.get("ph", 0),
            "tempWater": data.get("tempWater", data.get("tempW", 0)),
            "pump": data.get("pump", False),
            "fan": data.get("fan", False),
            "peri": data.get("peri", False),
            "led": data.get("led", False),
            "ledBrightness": data.get("ledBrightness", 100),
            "connected": data.get("connected", data.get("online", True)),
        }

        # Add to Readings collection
        doc_ref = db.collection(HTVCData.COLLECTION_READINGS).add(reading_data)[1]

        # Update latest reading in Devices collection
        db.collection(HTVCData.COLLECTION_DEVICES).document(device_id).set(
            {
                "latestReading": reading_data,
                "lastSeen": firestore.SERVER_TIMESTAMP,
                "floor": floor,
                "name": device_name,
                "status": "online",
            },
            merge=True
        )

        return doc_ref.id

    @staticmethod
    def save_reading_batch(
        device_id: str,
        device_name: str,
        readings: Dict[int, Dict[str, Any]]
    ) -> None:
        """
        Save multiple floor readings at once.

        Args:
            device_id: Unique device identifier
            device_name: Human-readable device name
            readings: Dict with floor numbers as keys and sensor data as values
        """
        if not _FIREBASE_AVAILABLE:
            return None

        db = get_db()
        batch = db.batch()

        for floor, data in readings.items():
            reading_data = {
                "deviceId": device_id,
                "deviceName": device_name,
                "floor": floor,
                "timestamp": firestore.SERVER_TIMESTAMP,
                "createdAt": datetime.utcnow().isoformat(),
                "tempAir": data.get("tempAir", data.get("tempA", 0)),
                "humidity": data.get("humidity", data.get("hum", 0)),
                "tds": data.get("tds", 0),
                "ph": data.get("ph", 0),
                "tempWater": data.get("tempWater", data.get("tempW", 0)),
                "pump": data.get("pump", False),
                "fan": data.get("fan", False),
                "peri": data.get("peri", False),
                "led": data.get("led", False),
            }

            doc_ref = db.collection(HTVCData.COLLECTION_READINGS).document()
            batch.set(doc_ref, reading_data)

        batch.commit()

    @staticmethod
    def get_latest_reading(device_id: str) -> Optional[Dict[str, Any]]:
        """
        Get the latest reading for a device.

        Args:
            device_id: Unique device identifier

        Returns:
            Dictionary with latest reading data or None
        """
        if not _FIREBASE_AVAILABLE:
            return None

        db = get_db()
        docs = (
            db.collection(HTVCData.COLLECTION_READINGS)
            .where("deviceId", "==", device_id)
            .order_by("timestamp", direction=firestore.Query.DESCENDING)
            .limit(1)
            .stream()
        )

        for doc in docs:
            data = doc.to_dict()
            data["id"] = doc.id
            return data

        return None

    @staticmethod
    def get_reading_history(
        device_id: str,
        floor: Optional[int] = None,
        limit: int = 100
    ) -> List[Dict[str, Any]]:
        """
        Get historical readings for a device.

        Args:
            device_id: Unique device identifier
            floor: Optional floor number to filter
            limit: Maximum number of readings to return

        Returns:
            List of reading dictionaries
        """
        if not _FIREBASE_AVAILABLE:
            return []

        db = get_db()
        query = db.collection(HTVCData.COLLECTION_READINGS).where(
            "deviceId", "==", device_id
        )

        if floor is not None:
            query = query.where("floor", "==", floor)

        query = query.order_by(
            "timestamp", direction=firestore.Query.DESCENDING
        ).limit(limit)

        results = []
        for doc in query.stream():
            data = doc.to_dict()
            data["id"] = doc.id
            results.append(data)

        return results

    @staticmethod
    def save_control_command(
        device_id: str,
        floor: int,
        target: str,
        value: Any,
        source: str = "dashboard"
    ) -> str:
        """
        Save a control command to Firestore.

        Args:
            device_id: Unique device identifier
            floor: Floor number
            target: Control target (led, pump, etc.)
            value: Control value
            source: Source of the command (dashboard, schedule, etc.)

        Returns:
            Document ID of the command
        """
        if not _FIREBASE_AVAILABLE:
            return "firebase-disabled"

        db = get_db()

        command_data = {
            "deviceId": device_id,
            "floor": floor,
            "target": target,
            "value": value,
            "source": source,
            "status": "pending",
            "timestamp": firestore.SERVER_TIMESTAMP,
        }

        doc_ref = db.collection(HTVCData.COLLECTION_CONTROL).add(command_data)[1]
        return doc_ref.id

    @staticmethod
    def get_pending_commands(device_id: str, floor: int) -> List[Dict[str, Any]]:
        """
        Get pending control commands for a device.

        Args:
            device_id: Unique device identifier
            floor: Floor number

        Returns:
            List of pending command dictionaries
        """
        if not _FIREBASE_AVAILABLE:
            return []

        db = get_db()
        docs = (
            db.collection(HTVCData.COLLECTION_CONTROL)
            .where("deviceId", "==", device_id)
            .where("floor", "==", floor)
            .where("status", "==", "pending")
            .order_by("timestamp", direction=firestore.Query.ASCENDING)
            .stream()
        )

        results = []
        for doc in docs:
            data = doc.to_dict()
            data["id"] = doc.id
            results.append(data)

        return results

    @staticmethod
    def mark_command_executed(command_id: str) -> None:
        """
        Mark a control command as executed.

        Args:
            command_id: Document ID of the command
        """
        if not _FIREBASE_AVAILABLE:
            return None

        db = get_db()
        db.collection(HTVCData.COLLECTION_CONTROL).document(command_id).set(
            {
                "status": "executed",
                "executedAt": firestore.SERVER_TIMESTAMP,
            },
            merge=True
        )

    @staticmethod
    def save_device_heartbeat(
        device_id: str,
        ip_address: str,
        rssi: int,
        free_heap: int,
        uptime: int,
    ) -> None:
        if not _FIREBASE_AVAILABLE:
            return None

        db = get_db()
        db.collection(HTVCData.COLLECTION_DEVICES).document(device_id).set(
            {
                "ip": ip_address,
                "rssi": rssi,
                "freeHeap": free_heap,
                "uptime": uptime,
                "lastSeen": firestore.SERVER_TIMESTAMP,
                "status": "online",
            },
            merge=True,
        )

    @staticmethod
    def save_device_heartbeat(
        device_id: str,
        ip_address: str,
        rssi: int,
        free_heap: int,
        uptime: int
    ) -> None:
        """
        Save device heartbeat/status to Firestore.

        Args:
            device_id: Unique device identifier
            ip_address: Device IP address
            rssi: WiFi signal strength
            free_heap: Free heap memory
            uptime: Device uptime in seconds
        """
        db = get_db()
        db.collection(HTVCData.COLLECTION_DEVICES).document(device_id).set(
            {
                "lastSeen": firestore.SERVER_TIMESTAMP,
                "ipAddress": ip_address,
                "wifiRssi": rssi,
                "freeHeap": free_heap,
                "uptime": uptime,
                "status": "online",
            },
            merge=True
        )

    @staticmethod
    def get_all_devices() -> List[Dict[str, Any]]:
        """
        Get all registered devices.

        Returns:
            List of device dictionaries
        """
        db = get_db()
        docs = db.collection(HTVCData.COLLECTION_DEVICES).stream()

        results = []
        for doc in docs:
            data = doc.to_dict()
            data["id"] = doc.id
            results.append(data)

        return results

    @staticmethod
    def save_log(
        device_id: str,
        level: str,
        message: str,
        details: Optional[Dict[str, Any]] = None
    ) -> None:
        """
        Save a log entry.

        Args:
            device_id: Unique device identifier
            level: Log level (info, warning, error)
            message: Log message
            details: Optional additional details
        """
        db = get_db()
        db.collection(HTVCData.COLLECTION_LOGS).add({
            "deviceId": device_id,
            "level": level,
            "message": message,
            "details": details or {},
            "timestamp": firestore.SERVER_TIMESTAMP,
        })
