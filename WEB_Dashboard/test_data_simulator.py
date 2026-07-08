#!/usr/bin/env python3
"""
Test Data Simulator - Simulates Mega data via MQTT for testing
Pushes test sensor data to MQTT topics to verify the entire pipeline
without needing actual Mega hardware.

Usage:
    python test_data_simulator.py
"""

import json
import time
import paho.mqtt.client as mqtt
from datetime import datetime

# MQTT Configuration
BROKER = "e898a641a7d3497aa75239be5bb34cd3.s1.eu.hivemq.cloud"
PORT = 8883
USERNAME = "HTVC_WEB"
PASSWORD = "Turauthongminh1"
CLIENT_ID = "test-simulator-001"

# Topics for 3 floors
TOPICS = {
    1: "htvc/sensors/floor1",
    2: "htvc/sensors/floor2",
    3: "htvc/sensors/floor3",
}

# Test data for 3 floors
TEST_DATA = {
    1: {
        "deviceId": "htvc-floor1-001",
        "deviceName": "Tủ Rau Thông Minh - Tầng 1",
        "floor": 1,
        "time": None,  # Will be set dynamically
        "tds": 1250,
        "tempA": 28.5,
        "hum": 65.3,
        "ph": 7.0,
        "tempW": 25.2,
        "pump": 0,
        "pTuoi": 0,
        "pAB": 0,
        "pCD": 0,
        "led": 1,
        "brightness": 100,
        "online": True,
    },
    2: {
        "deviceId": "htvc-floor2-001",
        "deviceName": "Tủ Rau Thông Minh - Tầng 2",
        "floor": 2,
        "time": None,
        "tds": 1300,
        "tempA": 29.0,
        "hum": 70.0,
        "ph": 6.8,
        "tempW": 26.0,
        "pump": 1,
        "pTuoi": 1,
        "pAB": 0,
        "pCD": 0,
        "led": 0,
        "brightness": 50,
        "online": True,
    },
    3: {
        "deviceId": "htvc-floor3-001",
        "deviceName": "Tủ Rau Thông Minh - Tầng 3",
        "floor": 3,
        "time": None,
        "tds": 1200,
        "tempA": 27.5,
        "hum": 60.0,
        "ph": 7.2,
        "tempW": 24.0,
        "pump": 0,
        "pTuoi": 0,
        "pAB": 1,
        "pCD": 0,
        "led": 0,
        "brightness": 0,
        "online": True,
    },
}


class TestSimulator:
    def __init__(self):
        self.client = mqtt.Client(client_id=CLIENT_ID, transport="tcp")
        self.client.username_pw_set(USERNAME, PASSWORD)
        self.client.tls_set()
        
        self.client.on_connect = self.on_connect
        self.client.on_disconnect = self.on_disconnect
        self.client.on_publish = self.on_publish
        
        self.connected = False
        self.published_count = 0

    def on_connect(self, client, userdata, flags, rc):
        if rc == 0:
            print(f"✅ Connected to MQTT broker ({BROKER}:{PORT})")
            self.connected = True
        else:
            print(f"❌ Connection failed (rc={rc})")

    def on_disconnect(self, client, userdata, rc):
        if rc != 0:
            print(f"⚠️  Unexpected disconnection (rc={rc})")
        self.connected = False

    def on_publish(self, client, userdata, mid):
        print(f"   └─ Published (message_id={mid})")

    def connect(self):
        """Connect to MQTT broker"""
        try:
            print(f"🔗 Connecting to {BROKER}:{PORT}...")
            self.client.connect(BROKER, PORT, 60)
            self.client.loop_start()
            time.sleep(2)  # Wait for connection
            return self.connected
        except Exception as e:
            print(f"❌ Error: {e}")
            return False

    def publish_test_data(self, floor: int, variation: int = 0):
        """
        Publish test data for a specific floor
        
        Args:
            floor: Floor number (1, 2, 3)
            variation: Optional variation to simulate changing values
        """
        if not self.connected:
            print("❌ Not connected to MQTT broker")
            return False

        data = TEST_DATA[floor].copy()
        data["time"] = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        
        # Add slight variation to values to simulate real sensor changes
        if variation > 0:
            data["tds"] += variation
            data["tempA"] += variation * 0.1
            data["hum"] += variation * 0.5
            data["ph"] += variation * 0.01
            data["tempW"] += variation * 0.05

        topic = TOPICS[floor]
        payload = json.dumps(data)
        
        print(f"\n📤 Publishing to {topic}")
        print(f"   ├─ Device: {data['deviceId']}")
        print(f"   ├─ Temp: {data['tempA']:.1f}°C")
        print(f"   ├─ Humidity: {data['hum']:.1f}%")
        print(f"   ├─ TDS: {data['tds']:.0f} ppm")
        print(f"   ├─ pH: {data['ph']:.2f}")
        print(f"   └─ LED: {'ON' if data['led'] else 'OFF'}, Pump: {'ON' if data['pump'] else 'OFF'}")

        try:
            self.client.publish(topic, payload, qos=1)
            self.published_count += 1
            return True
        except Exception as e:
            print(f"❌ Error publishing: {e}")
            return False

    def publish_all_floors(self, count: int = 5, interval: int = 5, variation_mode: bool = True):
        """
        Publish test data for all 3 floors multiple times
        
        Args:
            count: Number of times to publish
            interval: Seconds between publishes
            variation_mode: If True, add variation to simulate sensor drift
        """
        print(f"\n{'='*60}")
        print(f"🧪 TEST SIMULATOR - Publishing {count} batches of data")
        print(f"{'='*60}")
        
        for batch in range(1, count + 1):
            print(f"\n\n📍 BATCH {batch}/{count} - {datetime.now().strftime('%H:%M:%S')}")
            print("-" * 60)
            
            for floor in [1, 2, 3]:
                variation = (batch - 1) * 2 if variation_mode else 0
                self.publish_test_data(floor, variation)
                time.sleep(0.5)  # Small delay between floors
            
            if batch < count:
                print(f"\n⏳ Waiting {interval}s before next batch...")
                time.sleep(interval)
        
        print(f"\n\n{'='*60}")
        print(f"✅ DONE! Published {self.published_count} messages total")
        print(f"{'='*60}")
        print("\n📊 Data should now be visible on the web dashboard!")
        print("   → http://localhost:5000")
        print("\n✓ Check:")
        print("   1. Web dashboard shows sensor readings for 3 floors")
        print("   2. Firestore has readings-floor1, readings-floor2, readings-floor3")
        print("   3. Console logs show '[Sensors]' messages")

    def cleanup(self):
        """Cleanup and disconnect"""
        self.client.loop_stop()
        self.client.disconnect()


def main():
    print("\n")
    print("=" * 60)
    print("🧪 HTVC Test Data Simulator")
    print("=" * 60)
    print(f"Broker: {BROKER}:{PORT}")
    print(f"Floors: 1, 2, 3")
    print(f"Topics: htvc/sensors/floor[1-3]")
    print("=" * 60)

    simulator = TestSimulator()

    # Connect to broker
    if not simulator.connect():
        print("❌ Failed to connect to MQTT broker")
        return

    # Publish test data 5 times with 10-second intervals
    try:
        simulator.publish_all_floors(count=5, interval=10, variation_mode=True)
    except KeyboardInterrupt:
        print("\n\n⚠️  Interrupted by user")
    finally:
        simulator.cleanup()


if __name__ == "__main__":
    main()
