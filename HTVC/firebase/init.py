"""
firebase/init.py - Firebase initialization for HTVC project
"""

import os
import json
try:
    import firebase_admin
    from firebase_admin import credentials, firestore
except ModuleNotFoundError:
    firebase_admin = None
    credentials = None
    firestore = None

db = None

def init_firebase():
    """
    Initialize Firebase Admin SDK.
    Uses FIREBASE_CREDENTIALS_JSON env var (JSON string) or
    FIREBASE_SERVICE_ACCOUNT env var (path to JSON file).
    """
    global db
    if firebase_admin is None:
        raise RuntimeError("firebase-admin is not installed")
    if firebase_admin._apps:
        return

    cred_json = os.environ.get("FIREBASE_CREDENTIALS_JSON")
    if cred_json:
        cred = credentials.Certificate(json.loads(cred_json))
    else:
        path = os.environ.get(
            "FIREBASE_SERVICE_ACCOUNT",
            "serviceAccountKey.json"
        )
        cred = credentials.Certificate(path)

    firebase_admin.initialize_app(cred)
    db = firestore.client()


def get_db() -> firestore.Client:
    """
    Get Firestore client instance.
    Initializes Firebase if not already done.
    """
    global db
    if firebase_admin is None:
        raise RuntimeError("firebase-admin is not installed")
    if db is None:
        init_firebase()
        db = firestore.client()
    return db
