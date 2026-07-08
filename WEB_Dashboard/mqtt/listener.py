"""
mqtt/listener.py - MQTT listener for HTVC project
Listens for ESP32 sensor data and control commands via HiveMQ broker.
"""

import os
import json
import logging
import queue
import threading
from datetime import datetime
from typing import Optional, Callable
import paho.mqtt.client as mqtt

try:
    from firebase_admin import firestore
except ModuleNotFoundError:
    firestore = None

log = logging.getLogger("htvc.mqtt")

# HiveMQ Cloud Configuration
BROKER = os.environ.get("MQTT_BROKER", "e898a641a7d3497aa75239be5bb34cd3.s1.eu.hivemq.cloud")
PORT = int(os.environ.get("MQTT_PORT", "8883"))
MQTT_USER = os.environ.get("MQTT_USER", "HTVC_WEB")
MQTT_PASSWORD = os.environ.get("MQTT_PASSWORD", "Turauthongminh1")
USE_TLS = os.environ.get("MQTT_USE_TLS", "true").lower() == "true"

# Topic prefixes
TOPIC_PREFIX = "htvc"
TOPIC_SENSORS = f"{TOPIC_PREFIX}/sensors/+"  #htvc
TOPIC_CONTROL = f"{TOPIC_PREFIX}/control/+"
TOPIC_CONTROL_ACK = f"{TOPIC_PREFIX}/control/ack/+"
TOPIC_STATUS = f"{TOPIC_PREFIX}/status/+"
TOPIC_HEARTBEAT = f"{TOPIC_PREFIX}/heartbeat/+"

_client: Optional[mqtt.Client] = None
_callbacks = {}
_event_queues: list[queue.Queue] = []
_event_lock = threading.Lock()


def register_event_queue() -> queue.Queue:
    """Register a queue that receives MQTT events for SSE/WebSocket-like streaming."""
    event_queue = queue.Queue(maxsize=100)
    with _event_lock:
        _event_queues.append(event_queue)
    return event_queue


def unregister_event_queue(event_queue: queue.Queue) -> None:
    """Remove a previously registered event queue."""
    with _event_lock:
        if event_queue in _event_queues:
            _event_queues.remove(event_queue)


def _emit_event(event: dict) -> None:
    """Push an MQTT event to all registered stream listeners."""
    with _event_lock:
        queues_snapshot = list(_event_queues)

    for event_queue in queues_snapshot:
        try:
            event_queue.put_nowait(event)
        except queue.Full:
            try:
                event_queue.get_nowait()
            except queue.Empty:
                pass
            try:
                event_queue.put_nowait(event)
            except queue.Full:
                pass


def register_callback(topic_pattern: str, callback: Callable[[dict], None]) -> None:
    """
    Register a callback for messages matching the topic pattern.

    Args:
        topic_pattern: MQTT topic or pattern (e.g., "htvc/sensors/floor1")
        callback: Function to call with parsed message data
    """
    _callbacks[topic_pattern] = callback


def publish(topic: str, payload: dict, qos: int = 1) -> bool:
    """
    Publish a message to an MQTT topic.

    Args:
        topic: MQTT topic to publish to
        payload: Dictionary to publish as JSON
        qos: Quality of Service level (0, 1, or 2)

    Returns:
        True if successful, False otherwise
    """
    if _client is None or not _client.is_connected():
        log.warning("[MQTT] Client is not connected")
        return False

    try:
        if isinstance(payload, str):
            message = payload
        else:
            message = json.dumps(payload)
        result = _client.publish(topic, message, qos=qos)
        if result.rc == mqtt.MQTT_ERR_SUCCESS:
            log.info(f"[MQTT] Published to {topic}: {payload}")
            return True
        else:
            log.error(f"[MQTT] Publish failed with rc={result.rc}")
            return False
    except Exception as e:
        log.error(f"[MQTT] Publish error: {e}")
        return False


def _coerce_json_value(value):
    """Make sure payload values are JSON-serializable and stable."""
    if isinstance(value, bool):
        return value
    if isinstance(value, (int, float)):
        return value
    if value is None:
        return None
    return str(value)


def _coerce_command_value_text(value) -> str:
    """Normalize a control value into a compact string token."""
    if isinstance(value, bool):
        return "1" if value else "0"
    if isinstance(value, (int, float)):
        if isinstance(value, float) and value.is_integer():
            return str(int(value))
        return str(value)
    if value is None:
        return "0"
    text = str(value).strip()
    return text


def build_control_command_string(
    device_id: str,
    floor: int,
    target: str,
    value,
    command_id: str | None = None,
) -> str:
    """Build the compact MQTT command string understood by ESP32."""
    parts = [
        "HTVCCTRL",
        f"deviceId={device_id}",
        f"floor={int(floor)}",
        f"target={target}",
        f"value={_coerce_command_value_text(value)}",
    ]
    if command_id:
        parts.append(f"commandId={command_id}")
    return "|".join(parts)


def parse_control_command_string(raw: str) -> dict:
    """Parse the compact MQTT command string into a dictionary."""
    text = (raw or "").strip()
    if not text.startswith("HTVCCTRL|"):
        raise ValueError("Unsupported control command format")

    parts = text.split("|")
    payload = {"type": "control_command"}
    for part in parts[1:]:
        if "=" not in part:
            continue
        key, value = part.split("=", 1)
        key = key.strip()
        value = value.strip()
        if key == "floor":
            try:
                payload[key] = int(value)
            except ValueError:
                payload[key] = 1
        elif key == "value":
            lowered = value.lower()
            if lowered in {"1", "true", "on", "yes"}:
                payload[key] = True
            elif lowered in {"0", "false", "off", "no"}:
                payload[key] = False
            else:
                try:
                    payload[key] = int(value)
                except ValueError:
                    try:
                        payload[key] = float(value)
                    except ValueError:
                        payload[key] = value
        else:
            payload[key] = value
    return payload


def build_control_payload(device_id: str, floor: int, target: str, value, command_id: str | None = None) -> dict:
    """Build a normalized MQTT control payload."""
    payload = {
        "type": "control_command",
        "deviceId": str(device_id),
        "floor": int(floor),
        "target": str(target),
        "value": _coerce_json_value(value),
        "timestamp": datetime.utcnow().isoformat(),
    }

    if command_id:
        payload["commandId"] = str(command_id)

    return payload


def send_control_command(device_id: str, floor: int, target: str, value, command_id: str | None = None) -> bool:
    """Send a control command to a specific device via MQTT."""
    topic = f"{TOPIC_PREFIX}/control/floor{floor}"
    payload = build_control_command_string(device_id, floor, target, value, command_id)
    return publish(topic, payload)


def _parse_device_id_from_topic(topic: str) -> str:
    """Extract a fallback device identifier from topic path."""
    parts = topic.split("/")
    if len(parts) >= 4 and parts[2] == "ack":
        floor_part = parts[3]
        if floor_part.startswith("floor"):
            return f"htvc-{floor_part}-001"
        return floor_part
    if len(parts) >= 3:
        floor_part = parts[2]
        if floor_part.startswith("floor"):
            return f"htvc-{floor_part}-001"
        return floor_part
    return "unknown"


def _handle_sensors(device_id: str, data: dict) -> None:
    """Handle incoming sensor data."""
    try:
        from firebase.htvc_data import HTVCData

        payload_device_id = data.get("deviceId")
        if payload_device_id:
            device_id = str(payload_device_id)

        # Extract floor number from device_id
        floor = 1
        if "floor" in device_id.lower():
            try:
                floor = int(''.join(filter(str.isdigit, device_id)))
            except ValueError:
                floor = 1

        # Save reading to Firebase
        HTVCData.save_reading(
            device_id=device_id,
            device_name=data.get("deviceName", f"Tủ Rau - Tầng {floor}"),
            floor=floor,
            data=data.get("data", data)
        )

        log.info(f"[Sensors] {device_id}: "
                 f"Temp={data.get('tempAir', 0):.1f}C, "
                 f"Hum={data.get('humidity', 0):.1f}%, "
                 f"TDS={data.get('tds', 0):.0f}")

        # Call registered callbacks
        for pattern, callback in _callbacks.items():
            if "sensor" in pattern.lower():
                callback(data)

        _emit_event({
            "topic": "sensors",
            "deviceId": device_id,
            "payload": data,
        })

    except Exception as e:
        log.error(f"[Sensors] Error processing data: {e}")


def _handle_heartbeat(device_id: str, data: dict) -> None:
    """Handle incoming heartbeat/status data."""
    try:
        from firebase.htvc_data import HTVCData

        HTVCData.save_device_heartbeat(
            device_id=device_id,
            ip_address=data.get("ip", ""),
            rssi=data.get("rssi", 0),
            free_heap=data.get("free_heap", 0),
            uptime=data.get("uptime", 0)
        )

        _emit_event({
            "topic": "heartbeat",
            "deviceId": device_id,
            "payload": data,
        })

        log.debug(f"[Heartbeat] {device_id}: "
                  f"RSSI={data.get('rssi', 0)}, "
                  f"Uptime={data.get('uptime', 0)}s")

    except Exception as e:
        log.error(f"[Heartbeat] Error processing data: {e}")


def _handle_control(device_id: str, data: dict) -> None:
    """Handle incoming control acknowledgment."""
    if data.get("deviceId"):
        device_id = str(data["deviceId"])

    log.info(f"[Control] Ack from {device_id}: {data}")

    try:
        from firebase.htvc_data import HTVCData

        # Mark command as executed if there's an ID
        if "commandId" in data:
            HTVCData.mark_command_executed(data["commandId"])

        _emit_event({
            "topic": "control",
            "deviceId": device_id,
            "payload": data,
        })

    except Exception as e:
        log.error(f"[Control] Error processing ack: {e}")


def _handle_status(device_id: str, data: dict) -> None:
    """Handle incoming status messages."""
    log.info(f"[Status] {device_id}: {data.get('status', 'unknown')}")
    _emit_event({
        "topic": "status",
        "deviceId": device_id,
        "payload": data,
    })


# ── MQTT Callbacks ──────────────────────────────────────────────────────────

def _on_connect(client, userdata, flags, rc) -> None:
    """Callback when connection is established."""
    if rc == 0:
        log.info(f"[MQTT] Connected to {BROKER}:{PORT}")
        mqttConnected = True

        # Subscribe to all relevant topics
        client.subscribe(TOPIC_SENSORS, qos=1)
        client.subscribe(TOPIC_CONTROL_ACK, qos=1)
        client.subscribe(TOPIC_STATUS, qos=1)
        client.subscribe(TOPIC_HEARTBEAT, qos=1)

        log.info("[MQTT] Subscribed to HTVC sensors, control ack, status, and heartbeat topics")
    else:
        log.warning(
            "[MQTT] Connection failed with rc=%s broker=%s:%s user=%s tls=%s",
            rc,
            BROKER,
            PORT,
            MQTT_USER,
            USE_TLS,
        )

        if rc == 5:
            log.warning("[MQTT] rc=5 means bad username or password")
        elif rc == 7:
            log.warning("[MQTT] rc=7 means client is not authorized by the broker")


def _on_disconnect(client, userdata, rc) -> None:
    """Callback when disconnection occurs."""
    global mqttConnected
    mqttConnected = False
    if rc != 0:
        log.warning(f"[MQTT] Unexpected disconnection (rc={rc})")
    else:
        log.info("[MQTT] Disconnected gracefully")


def _on_message(client, userdata, msg) -> None:
    """Callback when a message is received."""
    try:
        # Parse JSON payload
        try:
            data = json.loads(msg.payload.decode("utf-8", "ignore"))
        except json.JSONDecodeError:
            raw_text = msg.payload.decode("utf-8", "ignore")
            try:
                data = parse_control_command_string(raw_text)
            except ValueError:
                data = {"raw": raw_text}

        topic = msg.topic
        device_id = _parse_device_id_from_topic(topic)

        log.debug(f"[MQTT] ← {topic}: {data}")

        # Route message based on topic
        if "/sensors/" in topic:
            _handle_sensors(device_id, data)
        elif "/heartbeat/" in topic:
            _handle_heartbeat(device_id, data)
        elif "/control/" in topic:
            _handle_control(device_id, data)
        elif "/status/" in topic:
            _handle_status(device_id, data)

    except Exception as e:
        log.error(f"[MQTT] Error processing message: {e}")


def _on_subscribe(client, userdata, mid, granted_qos) -> None:
    """Callback when subscription is confirmed."""
    log.debug(f"[MQTT] Subscribed (mid={mid}, qos={granted_qos})")


def _on_publish(client, userdata, mid) -> None:
    """Callback when publish is confirmed."""
    log.debug(f"[MQTT] Published (mid={mid})")


# ── Public API ──────────────────────────────────────────────────────────────

def start_mqtt() -> mqtt.Client:
    """
    Start the MQTT client and connect to the HiveMQ Cloud broker.

    Returns:
        The MQTT client instance
    """
    global _client

    client_id = os.environ.get("MQTT_CLIENT_ID", "htvc-flask-server")
    _client = mqtt.Client(client_id=client_id, transport="tcp")

    if MQTT_USER:
        _client.username_pw_set(MQTT_USER, MQTT_PASSWORD)

    # Enable TLS for HiveMQ Cloud
    if USE_TLS:
        _client.tls_set()
        log.info("[MQTT] TLS enabled for HiveMQ Cloud")

    _client.on_connect = _on_connect
    _client.on_disconnect = _on_disconnect
    _client.on_message = _on_message
    _client.on_subscribe = _on_subscribe
    _client.on_publish = _on_publish

    try:
        log.info(f"[MQTT] Connecting to {BROKER}:{PORT} (TLS: {USE_TLS})...")
        _client.connect(BROKER, PORT, 60)
        _client.loop_start()
        log.info("[MQTT] Client started")
    except Exception as e:
        log.error(f"[MQTT] Connection failed: {e}")

    return _client


def stop_mqtt() -> None:
    """Stop the MQTT client gracefully."""
    global _client
    if _client:
        _client.loop_stop()
        _client.disconnect()
        _client = None
        log.info("[MQTT] Client stopped")


def get_client() -> Optional[mqtt.Client]:
    """Get the current MQTT client instance."""
    return _client


def is_connected() -> bool:
    """Check if MQTT client is connected."""
    return _client is not None and _client.is_connected()
