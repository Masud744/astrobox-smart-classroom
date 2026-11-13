from contextlib import asynccontextmanager
from fastapi import FastAPI, HTTPException, UploadFile, File, Form, Request
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import FileResponse
from fastapi.staticfiles import StaticFiles
from pydantic import BaseModel, model_validator
from supabase import create_client, Client
from dotenv import load_dotenv
from typing import Optional
from datetime import datetime, timezone
import os
import shutil

load_dotenv()

db: Client = create_client(os.getenv("SUPABASE_URL"), os.getenv("SUPABASE_ANON_KEY"))

# Ensure uploads directory exists
UPLOAD_DIR = os.path.join(os.path.dirname(__file__), "uploads")
os.makedirs(UPLOAD_DIR, exist_ok=True)

# Dashboard build path (React Vite build)
DASHBOARD_DIST = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "dashboard", "dist"))
DASHBOARD_INDEX = os.path.join(DASHBOARD_DIST, "index.html")
DASHBOARD_ASSETS = os.path.join(DASHBOARD_DIST, "assets")

@asynccontextmanager
async def lifespan(app: FastAPI):
    print("FastRobox API starting...")
    yield
    print("FastRobox API stopped.")

app = FastAPI(title="FastRobox Smart Classroom API", version="1.1.0", lifespan=lifespan)

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

# Mount uploads directory for static binary distribution
app.mount("/uploads", StaticFiles(directory=UPLOAD_DIR), name="uploads")

# Mount dashboard static assets if built
if os.path.exists(DASHBOARD_ASSETS):
    app.mount("/assets", StaticFiles(directory=DASHBOARD_ASSETS), name="dashboard-assets")

# ── Pydantic Models ────────────────────────────────────────

class TelemetryPayload(BaseModel):
    device_id: str
    room_id: Optional[str] = "ROOM-01"
    seq: int
    ts: Optional[int] = None
    uptime_s: Optional[int] = None
    temp_c: Optional[float] = None
    rh: Optional[float] = None
    heat_index_c: Optional[float] = None
    flame: Optional[bool] = False
    door: Optional[str] = "LOCKED"
    status: Optional[str] = "SAFE"
    heat_trend: Optional[str] = None
    rssi: Optional[int] = None
    heap_kb: Optional[int] = None
    chip_temp_c: Optional[float] = None
    fw: Optional[str] = "1.0.0"
    quality: Optional[str] = "good"

    @model_validator(mode="before")
    @classmethod
    def normalize_keys(cls, data):
        if isinstance(data, dict):
            mapping = {
                "deviceId": "device_id",
                "roomId": "room_id",
                "uptimeS": "uptime_s",
                "tempC": "temp_c",
                "heatIndexC": "heat_index_c",
                "heatTrend": "heat_trend",
                "heapKb": "heap_kb",
                "chipTempC": "chip_temp_c",
            }
            for camel, snake in mapping.items():
                if camel in data and snake not in data:
                    data[snake] = data[camel]
        return data

class EventPayload(BaseModel):
    device_id: Optional[str] = None
    room_id: Optional[str] = "ROOM-01"
    seq: Optional[int] = None
    ts: Optional[int] = None
    type: str
    detail: Optional[dict] = None

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
    alert_type: str    # flame | panic | temp_high | hum_high | offline | unknown_card | unknown_face

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

# ── Health & Dashboard ─────────────────────────────────────

@app.get("/")
def health():
    return {"status": "ok", "project": "fastrobox-classroom", "version": "1.1.0"}

@app.get("/dashboard")
def serve_dashboard():
    if os.path.exists(DASHBOARD_INDEX):
        return FileResponse(DASHBOARD_INDEX)
    fallback = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", "dashboard", "index.html"))
    if os.path.exists(fallback):
        return FileResponse(fallback)
    raise HTTPException(status_code=404, detail="Dashboard file not found")

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
    updates["updated_at"] = datetime.now(timezone.utc).isoformat()
    res = db.table("settings").update(updates).eq("id", 1).execute()
    return res.data[0] if res.data else {"ok": True}

# ── Telemetry & Events (Level 2) ───────────────────────────

@app.post("/api/telemetry")
def post_telemetry(payload: TelemetryPayload):
    # Enforce deduplication by (device_id, seq) — returns 409 Conflict if replayed
    existing = db.table("telemetry").select("id")\
        .eq("device_id", payload.device_id)\
        .eq("seq", payload.seq).execute()
    if existing.data:
        raise HTTPException(status_code=409, detail="Duplicate seq - replay rejected")

    res = db.table("telemetry").insert(payload.model_dump()).execute()
    return {"ok": True, "id": res.data[0]["id"] if res.data else None}

@app.get("/api/telemetry/latest")
def get_telemetry_latest(room_id: str = "ROOM-01"):
    res = db.table("telemetry").select("*").eq("room_id", room_id)\
        .order("logged_at", desc=True).limit(1).execute()
    if not res.data:
        raise HTTPException(status_code=404, detail="No telemetry data yet")
    return res.data[0]

@app.get("/api/telemetry/history")
def get_telemetry_history(room_id: str = "ROOM-01", limit: int = 50):
    res = db.table("telemetry").select("*").eq("room_id", room_id)\
        .order("logged_at", desc=True).limit(limit).execute()
    return res.data

@app.post("/api/event")
def post_event(payload: EventPayload):
    res = db.table("events").insert(payload.model_dump()).execute()
    return {"ok": True, "id": res.data[0]["id"] if res.data else None}

@app.get("/api/events")
def get_events(limit: int = 50, event_type: Optional[str] = None):
    query = db.table("events").select("*").order("logged_at", desc=True)
    if event_type:
        query = query.eq("type", event_type)
    res = query.limit(limit).execute()
    return res.data

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
    return {"update": True, "version": fw["version"], "url": fw["binary_url"], "notes": fw.get("notes", "")}

@app.post("/api/ota/upload")
async def upload_ota(
    request: Request,
    file: UploadFile = File(...),
    version: str = Form(...),
    notes: Optional[str] = Form("Dashboard OTA update")
):
    if not file.filename.endswith(".bin"):
        raise HTTPException(status_code=400, detail="Only .bin firmware files allowed")

    file_path = os.path.join(UPLOAD_DIR, file.filename)
    with open(file_path, "wb") as buffer:
        shutil.copyfileobj(file.file, buffer)

    base_url = str(request.base_url).rstrip("/")
    binary_url = f"{base_url}/uploads/{file.filename}"

    # Deactivate previously active firmware updates
    db.table("ota_firmware").update({"active": False}).eq("active", True).execute()

    # Record new active firmware
    db.table("ota_firmware").insert({
        "version": version,
        "binary_url": binary_url,
        "notes": notes,
        "active": True
    }).execute()

    return {
        "ok": True,
        "version": version,
        "filename": file.filename,
        "url": binary_url
    }

# ── Direct Execution ───────────────────────────────────────

if __name__ == "__main__":
    import uvicorn
    port = int(os.getenv("PORT", 8000))
    uvicorn.run("main:app", host="0.0.0.0", port=port, reload=True)
