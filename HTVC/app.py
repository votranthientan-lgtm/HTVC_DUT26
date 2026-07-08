"""
app.py - HTVC Flask Application
Tủ Rau Thông Minh - Smart Vegetable Rack System
"""

import os
import logging
from flask import Flask
from flask_cors import CORS
from dotenv import load_dotenv

load_dotenv()

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s [%(levelname)s] %(name)s: %(message)s",
)

def create_app() -> Flask:
    """Create and configure the Flask application."""
    app = Flask(__name__)
    # Accept both '/path' and '/path/' forms to avoid accidental 404s from trailing slash.
    app.url_map.strict_slashes = False
    app.secret_key = os.environ.get("SECRET_KEY", "htvc-secret-key-change-in-production")
    app.config["SESSION_COOKIE_HTTPONLY"] = True
    app.config["SESSION_COOKIE_SAMESITE"] = "Lax"

    # Enable CORS
    CORS(app, supports_credentials=True)

    # Initialize Firebase
    from firebase.init import init_firebase
    try:
        init_firebase()
        logging.info("[Firebase] Initialized successfully")
    except Exception as e:
        logging.warning(f"[Firebase] Initialization skipped: {e}")

    # Register blueprints
    from blueprints.dashboard.routes import bp as dashboard_bp
    from blueprints.api.routes import bp as api_bp
    from blueprints.student.routes import bp as student_bp

    app.register_blueprint(dashboard_bp)
    app.register_blueprint(api_bp)
    app.register_blueprint(student_bp)

    return app


if __name__ == "__main__":
    from mqtt.listener import start_mqtt

    app = create_app()

    # Start MQTT listener
    try:
        start_mqtt()
        logging.info("[MQTT] Listener started")
    except Exception as e:
        logging.warning(f"[MQTT] Listener start skipped: {e}")

    # Run the Flask app
    app.run(host="0.0.0.0", port=5000, debug=True)
