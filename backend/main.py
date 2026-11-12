from contextlib import asynccontextmanager
from fastapi import FastAPI, HTTPException
from fastapi.middleware.cors import CORSMiddleware
from pydantic import BaseModel
from supabase import create_client, Client
from dotenv import load_dotenv
from typing import Optional
import os

load_dotenv()

db: Client = create_client(os.getenv("SUPABASE_URL"), os.getenv("SUPABASE_ANON_KEY"))

@asynccontextmanager
async def lifespan(app: FastAPI):
    print("FastRobox API starting...")
    yield
    print("FastRobox API stopped.")

app = FastAPI(title="FastRobox Smart Classroom API", version="1.0.0", lifespan=lifespan)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

# ── Pydantic models ────────────────────────────────────────

class SensorPayload(BaseModel):
    room_id: str
    temperature: Optional[float] = None
    humidity: Optional[float] = None
    flame: bool = False
    wifi_rssi: Optional[int] = None

class AttendancePayload(BaseModel):
    room_id: str
    card_uid: str
    person_type: str   # student | teacher | unknown
    action: str        # check_in | check_out | denied
    method: str        # rfid | face | manual
    face_label: Optional[str] = None

class DoorPayload(BaseModel):
    room_id: str
    state: str         # locked | unlocked
    trigger: str       # rfid | dashboard | auto | panic

class AlertPayload(BaseModel):
    room_id: str
    alert_type: str    # flame | panic | temp_high | hum_high | offline | unknown_face

class HeartbeatPayload(BaseModel):
    room_id: str
    firmware_version: Optional[str] = None
    ip_address: Optional[str] = None
    uptime_sec: Optional[int] = None

class SettingsPatch(BaseModel):
    room_id: Optional[str] = None
    class_start: Optional[str] = None
    temp_max: Optional[float] = None
    hum_max: Optional[float] = None
    buzzer_enabled: Optional[bool] = None
    door_locked: Optional[bool] = None

# ── Health ─────────────────────────────────────────────────

@app.get("/")
def health():
    return {"status": "ok", "project": "fastrobox-classroom"}

# ── Settings ───────────────────────────────────────────────

@app.get("/api/settings")
def get_settings():
    res = db.table("settings").select("*").eq("id", 1).single().execute()
    if not res.data:
        raise HTTPException(status_code=404, detail="Settings not found")
    return res.data

@app.patch("/api/settings")
def patch_settings(payload: SettingsPatch):
    updates = payload.model_dump(exclude_none=True)
    if not updates:
        raise HTTPException(status_code=400, detail="No fields to update")
    from datetime import datetime, timezone
    updates["updated_at"] = datetime.now(timezone.utc).isoformat()
    res = db.table("settings").update(updates).eq("id", 1).execute()
    return res.data[0] if res.data else {"ok": True}

# ── Sensor ─────────────────────────────────────────────────

@app.post("/api/sensor")
def post_sensor(payload: SensorPayload):
    db.table("sensor_logs").insert(payload.model_dump()).execute()
    return {"ok": True}

@app.get("/api/sensor/latest")
def get_sensor_latest(room_id: str = "ROOM-01"):
    res = db.table("sensor_logs").select("*").eq("room_id", room_id)\
        .order("logged_at", desc=True).limit(1).execute()
    if not res.data:
        raise HTTPException(status_code=404, detail="No sensor data yet")
    return res.data[0]

@app.get("/api/sensor/history")
def get_sensor_history(room_id: str = "ROOM-01", limit: int = 50):
    res = db.table("sensor_logs").select("*").eq("room_id", room_id)\
        .order("logged_at", desc=True).limit(limit).execute()
    return res.data

# ── Attendance ─────────────────────────────────────────────

@app.post("/api/attendance")
def post_attendance(payload: AttendancePayload):
    db.table("attendance").insert(payload.model_dump()).execute()
    return {"ok": True}

@app.get("/api/attendance")
def get_attendance(room_id: str = "ROOM-01", search: str = "", limit: int = 100):
    query = db.table("attendance").select("*").eq("room_id", room_id)\
        .order("logged_at", desc=True).limit(limit)
    if search:
        query = query.ilike("card_uid", f"%{search}%")
    return query.execute().data

# ── Door ───────────────────────────────────────────────────

@app.post("/api/door")
def post_door(payload: DoorPayload):
    db.table("settings").update({"door_locked": payload.state == "locked"})\
        .eq("id", 1).execute()
    db.table("door_events").insert(payload.model_dump()).execute()
    return {"ok": True}

@app.get("/api/door/history")
def get_door_history(room_id: str = "ROOM-01"):
    res = db.table("door_events").select("*").eq("room_id", room_id)\
        .order("logged_at", desc=True).limit(50).execute()
    return res.data

# ── Alerts ─────────────────────────────────────────────────

@app.post("/api/alert")
def post_alert(payload: AlertPayload):
    db.table("alerts").insert(payload.model_dump()).execute()
    return {"ok": True}

@app.get("/api/alerts")
def get_alerts(room_id: str = "ROOM-01"):
    res = db.table("alerts").select("*").eq("room_id", room_id)\
        .order("logged_at", desc=True).limit(50).execute()
    return res.data

# ── Heartbeat ──────────────────────────────────────────────

@app.post("/api/heartbeat")
def post_heartbeat(payload: HeartbeatPayload):
    db.table("heartbeat").insert(payload.model_dump()).execute()
    return {"ok": True}

# ── OTA ────────────────────────────────────────────────────

@app.get("/api/ota/latest")
def get_ota_latest():
    res = db.table("ota_firmware").select("*").eq("active", True)\
        .order("created_at", desc=True).limit(1).execute()
    if not res.data:
        return {"update": False}
    fw = res.data[0]
    return {"update": True, "version": fw["version"], "url": fw["binary_url"], "notes": fw["notes"]}

