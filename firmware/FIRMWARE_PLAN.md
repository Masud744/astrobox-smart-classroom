# Firmware Plan — FastRobox Smart Classroom

## Board
- **ESP32-S3-CAM** (ESP32-S3-WROOM-1-N16R8)
- Flash: 16 MB | PSRAM: 8 MB OPI | CPU: Dual-core LX7 @ 240 MHz

## Arduino IDE Settings (Critical)
```
Board:            ESP32S3 Dev Module
Flash Size:       16MB (128Mb)
Partition Scheme: Min SPIFFS (1.9MB APP with OTA)
PSRAM:            OPI PSRAM        ← MUST be enabled or face-rec crashes
CPU Frequency:    240 MHz
Upload Speed:     921600
```

---

## FreeRTOS Architecture

We use **implicit FreeRTOS** (via Arduino loop) for most tasks,
and **one explicit FreeRTOS task** for the camera/face pipeline on Core 1.

```
┌─────────────────────────────────────────────────────┐
│  Core 0 (Arduino loop — implicit FreeRTOS)          │
│                                                     │
│  SmartProv.handle()      — WiFi provisioning        │
│  localServer.handleClient()— Local HTTP page        │
│  readSensors()           — DHT11 every 5s           │
│  checkFlame()            — IR sensor, buzzer alert  │
│  checkTouch()            — Panic pad                │
│  checkRFID()             — RC522 card scan          │
│  pollSettings()          — Cloud settings every 10s │
│  postSensor()            — Cloud POST every 5s      │
│  postHeartbeat()         — Cloud POST every 30s     │
└─────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────┐
│  Core 1 (xTaskCreatePinnedToCore)                   │
│                                                     │
│  cameraTask()                                       │
│    ├── MJPEG stream (served via local HTTP)         │
│    ├── Face detection (esp-dl)                      │
│    └── Face recognition → triggers attendance POST  │
└─────────────────────────────────────────────────────┘

Shared state between cores:
  → volatile bool faceDetected
  → volatile char lastFaceLabel[32]
  → SemaphoreHandle_t faceSemaphore  (mutex for shared data)
```

---

## File Structure

```
firmware/main/
├── main.ino          ← setup(), loop(), FreeRTOS task launch
├── config.h          ← pin definitions, intervals, firmware version
├── wifi_prov.cpp/h   ← SmartProv wrapper (WiFi + custom fields)
├── sensors.cpp/h     ← DHT11, IR flame, touch pad
├── rfid.cpp/h        ← RC522 RFID: read UID, NVS role lookup
├── door.cpp/h        ← door state (locked/unlocked), LED + cloud sync
├── cloud.cpp/h       ← HTTPS POST/GET to backend API
├── local_server.cpp/h← WebServer on port 80, status page + /status.json
├── camera.cpp/h      ← OV2640 init, MJPEG stream, frame capture
├── face.cpp/h        ← face enroll, face recognition (Core 1 task)
├── ota.cpp/h         ← OTA: poll /api/ota/latest, HTTPUpdate flash
├── offline.cpp/h     ← SPIFFS queue: save failed POSTs, flush on reconnect
└── data/
    └── index.html    ← local status page HTML (served from SPIFFS)
```

---

## Module Details

### wifi_prov (.cpp/.h)
- Uses SmartProv library
- Captive portal fields: WiFi SSID/pass, Cloud URL, Room ID, Class start time
- On connect: saves all fields to NVS via SmartProv
- Fallback: if previously connected, auto-reconnect from NVS
- Offline mode flag set if WiFi drops during operation

### sensors (.cpp/.h)
- DHT11: read temp + humidity, retry once on NaN
- IR Flame: digitalRead (active LOW), debounce 200ms
- Touch pad: touchRead() < threshold (40), debounce
- All reads non-blocking (no delay())

### rfid (.cpp/.h)
- RC522 via SPI
- Read UID on card present
- NVS lookup: `rfid/<UID>` → role (student/teacher/unknown)
- Register card: called from local HTTP endpoint `/rfid/register`
  ```
  POST /rfid/register
  body: { "uid": "AA:BB:CC:DD", "role": "student" }
  ```
- Beep pattern: 1 beep = success, 2 beeps = denied

### door (.cpp/.h)
- State stored in-memory: `bool doorLocked`
- LED: red = locked, green = unlocked
- Sync: on state change → POST /api/door
- Poll: settings.door_locked checked every 10s (dashboard control)
- Trigger sources: rfid, dashboard, auto, panic

### cloud (.cpp/.h)
- All HTTP calls non-blocking with timeout (5s)
- If WiFi down: returns false, caller queues to SPIFFS offline buffer
- Endpoints used:
  - POST /api/sensor
  - POST /api/attendance
  - POST /api/door
  - POST /api/alert
  - POST /api/heartbeat
  - GET  /api/settings
  - GET  /api/ota/latest

### local_server (.cpp/.h)
- WebServer on port 80
- Routes:
  - `GET /`             → status HTML page (auto-refresh 3s)
  - `GET /status.json`  → machine-readable JSON
  - `GET /stream`       → redirect to camera stream port 81
  - `POST /rfid/register` → register RFID card role to NVS
  - `POST /door`        → toggle door (local control)

### camera (.cpp/.h)
- Runs on Core 1 via xTaskCreatePinnedToCore (stack: 8192, priority: 1)
- Init: FRAMESIZE_SVGA (800×600) for recognition, FRAMESIZE_QVGA for stream
- MJPEG stream: port 81, `/stream` path
- Frame capture: shared buffer protected by semaphore
- Hands frame to face.cpp for processing

### face (.cpp/.h)
- Uses esp-dl face detection + recognition
- **Enroll flow** (competition setup, done once):
  - Trigger: BOOT button held 3s OR POST /face/enroll?label=StudentName
  - Captures 5 frames, averages embedding, saves to NVS
- **Recognition flow** (continuous):
  - Runs in camera task on Core 1
  - On match (confidence > 0.8): POST /api/attendance (method: face)
  - On unknown face: POST /api/alert (alert_type: unknown_face)
  - Cooldown: 10s between same-face recognitions
- **Cleanup**: all face embeddings cleared from NVS on demand
  - `POST /face/clear` or BOOT button held 10s

### ota (.cpp/.h)
- Polls GET /api/ota/latest every 30 min (or on BOOT button press)
- If update available: HTTPUpdate.update(url)
- Shows progress on Serial + local page
- Reboots after successful flash

### offline (.cpp/.h)
- SPIFFS-based queue: `/offline_queue.jsonl`
- On failed POST: appends JSON line to file
- On WiFi reconnect: reads file line by line, retries each POST
- Max queue size: 500 entries (~50KB)
- On flush success: deletes file

---

## Memory Budget

```
Flash (16 MB total):
  Bootloader + partition table:   ~28 KB
  NVS (SmartProv + RFID + faces): ~64 KB
  Firmware binary:              ~2,600 KB
  OTA partition (mirror):       ~2,600 KB
  SPIFFS (local page + queue):  ~1,000 KB
  ─────────────────────────────────────────
  Used: ~6,300 KB | Free: ~9,900 KB ✅

RAM — Internal SRAM (512 KB):
  Arduino + WiFi stack:   ~300 KB
  Sensor structs, misc:   ~20 KB
  ─────────────────────────────────────────
  Used: ~320 KB | Free: ~192 KB ✅

RAM — External PSRAM (8 MB):
  Camera frame buffer:    ~100 KB
  Face rec tensors:       ~3,000 KB
  Face embeddings:        ~100 KB
  HTTP buffers:           ~50 KB
  ─────────────────────────────────────────
  Used: ~3,250 KB | Free: ~4,850 KB ✅
```

---

## Libraries Required (Arduino Library Manager)

| Library | Purpose |
|---------|---------|
| SmartProv | WiFi captive portal + NVS config |
| MFRC522 | RC522 RFID reader |
| DHT sensor library (Adafruit) | DHT11 temperature/humidity |
| ArduinoJson | JSON build/parse for cloud API |
| esp32-camera | Built into ESP32 Arduino core |
| ESP-DL | Face detection + recognition (from Espressif GitHub) |

---

## Competition Day Firmware Checklist

Before calling judges:
- [ ] SmartProv portal tested with unknown SSID
- [ ] All 5 display surfaces open: Serial, Local page, Dashboard, Buzzer/LED, DB
- [ ] RFID cards registered (after receiving hardware)
- [ ] Faces enrolled (team members, done once)
- [ ] Response times measured locally (temp alert, door unlock, attendance log)
- [ ] Offline mode tested: unplug WiFi → data queued → reconnect → flushed
- [ ] OTA tested: upload dummy .bin via dashboard → ESP32 flashes → reboots

## Notes for Other Agents

- Do NOT hardcode WiFi, Room ID, class start time — these come from SmartProv portal
- Do NOT commit face embeddings or card UIDs — these are set up on competition day
- All cross-core shared state must use `volatile` or a semaphore
- PSRAM must be allocated explicitly: `heap_caps_malloc(size, MALLOC_CAP_SPIRAM)`
- Camera and face recognition MUST run on Core 1 to avoid blocking Core 0
- Use `millis()` timers everywhere — never `delay()` in main loop
