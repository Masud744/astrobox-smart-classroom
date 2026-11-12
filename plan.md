# FastRobox 1.0 — Project Plan
**AI-Powered Smart Classroom: Safety, Access and Vision**
BUBT Techathon Grand Finale | 7 Hours | 500 Points | 12 Levels

---

## Architecture Overview

```
[ESP32-S3-CAM]
  ├── DHT11 (temp/humidity)
  ├── IR Flame Sensor
  ├── RC522 RFID Reader
  ├── Active Buzzer
  ├── Touch Wire (panic pad)
  └── Onboard RGB LED + Camera

      ↕ HTTPS (WiFi)

[Cloud Backend] ←→ [Supabase DB]
      ↕
[Web Dashboard] ← browser/phone
```

---

## Phase 1 — Foundation (Pre-competition)

**Goal:** WiFi provisioning + cloud connection working end-to-end.

### 1.1 Repo & Git Setup
- [ ] Folder structure created
- [ ] `.gitignore` configured (no secrets, no brain files)
- [ ] `README.md` base written
- [ ] `.env.example` added with all required keys

### 1.2 Cloud Backend
- [ ] Supabase project created
- [ ] Database schema designed and migrated
- [ ] REST API endpoints deployed (Vercel/Railway)
- [ ] HTTPS public URL confirmed working
- [ ] Remote settings endpoint (`GET /settings`)
- [ ] Data ingestion endpoint (`POST /data`)
- [ ] History/log query endpoint (`GET /history`)

### 1.3 Web Dashboard
- [ ] Live sensor data display (temp, humidity, flame status)
- [ ] Door status display (LOCKED / UNLOCKED)
- [ ] Attendance log table (searchable)
- [ ] Remote settings panel (change Room ID, thresholds, class time)
- [ ] Camera feed embed
- [ ] WiFi signal strength display

### 1.4 ESP32 Firmware Base
- [ ] SmartProv captive portal (WiFi + Room ID + Class time — no hardcoding)
- [ ] Cloud POST on boot (device heartbeat)
- [ ] Local HTTP status page (offline fallback at `192.168.x.x`)
- [ ] NTP time sync
- [ ] Offline mode: queue data to SPIFFS, flush when back online

---

## Phase 2 — Sensor Integration (Pre-competition)

**Goal:** All 6 hardware components coded and tested individually.

### 2.1 DHT11
- [ ] Read temp + humidity every N seconds
- [ ] POST to cloud
- [ ] Show on serial, local page, dashboard

### 2.2 IR Flame Sensor
- [ ] Digital read on DO pin
- [ ] Trigger buzzer + RGB LED on flame detection
- [ ] Alert POST to cloud within time limit
- [ ] Dashboard shows flame status live

### 2.3 RC522 RFID
- [ ] White card → student check-in
- [ ] Blue key-tag → teacher activity
- [ ] Unknown card → denied + log
- [ ] POST attendance to cloud
- [ ] Dashboard shows attendance list

### 2.4 Active Buzzer
- [ ] Short beep on RFID scan (success/fail patterns)
- [ ] Continuous alarm on flame
- [ ] Dashboard can toggle buzzer remotely

### 2.5 Touch Wire (Panic Pad)
- [ ] Touch pin detection
- [ ] Panic alert POST to cloud instantly
- [ ] Dashboard shows panic event

### 2.6 Camera
- [ ] MJPEG stream endpoint on ESP32
- [ ] Dashboard embeds live stream
- [ ] Capture frame for face recognition

---

## Phase 3 — Advanced Features (Pre-competition)

**Goal:** Face recognition, door simulation, OTA structure ready.

### 3.1 Face Recognition
- [ ] Register faces (team members = students)
- [ ] Recognition triggers attendance
- [ ] Unknown face → alert
- [ ] All face data deleted after competition

### 3.2 Door Simulation
- [ ] State stored in firmware (LOCKED / UNLOCKED)
- [ ] RGB LED shows state (red = locked, green = unlocked)
- [ ] Local page shows state
- [ ] Dashboard shows + can toggle
- [ ] RFID unlock logic (teacher card = unlock, student = check-in only)

### 3.3 Remote Settings
- [ ] ESP32 polls `/settings` every N seconds
- [ ] Settings: temp threshold, Room ID, class start time, buzzer enable
- [ ] Dashboard can push new settings
- [ ] No restart required

### 3.4 OTA Firmware Update
- [ ] Dashboard has "Upload .bin" button
- [ ] Backend stores binary and serves it
- [ ] ESP32 fetches and flashes via `HTTPUpdate.h`
- [ ] Progress shown on serial + local page

---

## Phase 4 — Integration & Testing (Pre-competition)

**Goal:** Full system working end-to-end before competition day.

- [ ] All sensors integrated in one firmware
- [ ] All 5 display surfaces working simultaneously:
  - Serial Monitor
  - Local status page
  - Web dashboard
  - Light & buzzer
  - Database records
- [ ] Offline fallback tested (disconnect WiFi → reconnect → data syncs)
- [ ] SmartProv tested with unknown SSID (simulate judge giving WiFi)
- [ ] Response times measured and within limits
- [ ] Git tags structure practiced (`level-01`, `level-02`, ...)
- [ ] Write-up template filled for a mock level

---

## Phase 5 — Competition Day (On-site, 7 Hours)

**Goal:** Pass as many levels as possible. Max 4 judging calls.

### Judging Call Strategy

| Call | Levels | When |
|------|--------|------|
| Call 1 | Level 1, 2, 3 | Hour ~2 (after hardware setup + SmartProv config) |
| Call 2 | Level 4, 5, 6 | Hour ~3.5 |
| Call 3 | Level 7, 8, 9 | Hour ~5 |
| Call 4 | Level 10, 11, 12 | Hour ~6.5 (keep buffer!) |

### On-site Checklist (Before Each Judging Call)
- [ ] Tested locally 10+ times
- [ ] All 5 display surfaces open and visible
- [ ] Video recording started (stopwatch visible)
- [ ] Team ID and Level number ready to announce aloud
- [ ] Write-up started for this level

### Post-Level Checklist (Within 30 Minutes of Judging)
- [ ] Video exported (max 10 min, max 3 clips, 720p+)
- [ ] Write-up PDF completed (5 sections, max 2 pages)
- [ ] Git committed and tagged (`git tag level-NN && git push --tags`)
- [ ] Files uploaded: `<TeamID>_L<NN>_video.mp4`, `<TeamID>_L<NN>_writeup.pdf`

---

## Folder Structure

```
astrobox-smart-classroom/
├── firmware/
│   ├── main/
│   │   ├── main.ino
│   │   ├── config.h          # pin definitions, constants (no secrets)
│   │   ├── wifi_prov.cpp/h   # SmartProv wrapper
│   │   ├── sensors.cpp/h     # DHT11, flame, touch
│   │   ├── rfid.cpp/h        # RC522
│   │   ├── camera.cpp/h      # stream + face
│   │   ├── door.cpp/h        # door state simulation
│   │   ├── cloud.cpp/h       # HTTPS POST/GET
│   │   ├── local_server.cpp/h # local status page
│   │   └── ota.cpp/h         # OTA update
│   └── data/                 # SPIFFS files (offline queue, local page HTML)
├── backend/
│   ├── api/                  # API routes
│   ├── supabase/
│   │   └── schema.sql        # DB schema
│   └── .env.example          # required env vars (no actual secrets)
├── dashboard/
│   ├── index.html
│   ├── style.css
│   └── app.js
├── writeup-template/
│   └── template.md           # 5-section write-up template
├── plan.md                   # this file
├── README.md
└── .gitignore
```

---

## Key Rules to Never Forget

| Rule | Consequence |
|------|-------------|
| No hardcoded WiFi/Room ID/Class time | Instant fail |
| No secrets in code or git | Disqualification |
| Max 4 judging calls total | Plan carefully |
| 2 attempts per level max | Don't rush |
| Both attempts fail = all future levels closed | Most dangerous rule |
| Evidence must be uploaded within 30 min | Score becomes 0 |
| 1st attempt pass = 100%, 2nd = 70% | Test before calling |

