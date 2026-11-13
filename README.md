# FastRobox AI-Powered Smart Classroom

**AI-Powered Smart Classroom: Safety, Access and Vision Challenge**  
*FastRobox 1.0 Techathon — Grand Finale | Bangladesh University of Business and Technology (BUBT)*  
**Team ID:** `T10` | **Device ID:** `T10-D1`

---

## 1. System Overview & Architecture

The FastRobox Smart Classroom platform is an end-to-end IoT, safety, and vision management system running on a single **ESP32-S3-CAM** microcontroller interfaced with a secure, cloud-hosted **Python FastAPI & Supabase** backend platform and live **React/Tailwind** dashboard.

```
+───────────────────────────────────────────────────────────────────────────────────+
│                              ESP32-S3-CAM (Team T10)                              │
│                                                                                   │
│  Core 0 (Arduino Main & Realtime Control):                                        │
│   ├── FreeRTOS 2 Hz Diagnostic LED (Green/Yellow/Red/Magenta/Blue)                │
│   ├── DHT11 Temp/Humidity & Rothfusz Heat Index calculation                      │
│   ├── 2s Flame Debounce & Fail-Safe Door Relay (NVS Fire State Persistence)       │
│   ├── Dual Capacitive Touch Panic Pad (GPIO 4 & GPIO 1)                           │
│   ├── RC522 RFID Card Authentication & 5s Auto-Relock                             │
│   ├── SmartProv Captive Portal SoftAP (SmartClass-T10 @ 192.168.4.1)              │
│   ├── mDNS Responder (http://smartclass-t10.local)                                │
│   ├── LittleFS WebServer (Live local status page, auto-updates every 2s)          │
│   └── Monotonic Seq NVS Telemetry Client (5 ± 1s JSON over HTTPS)                 │
│                                                                                   │
│  Core 1 (Vision & Streaming):                                                     │
│   ├── OV2640 MJPEG Camera Stream Server (:81/stream)                              │
│   └── Real-time Face Detection & Recognition Task                                 │
+────────────────────────────────────────┬──────────────────────────────────────────+
                                         │ HTTPS / TLS (5s Telemetry & Realtime Events)
                                         ▼
+───────────────────────────────────────────────────────────────────────────────────+
│                               Live Cloud Platform                                 │
│                                                                                   │
│  FastAPI Backend (Python 3.10+):                                                  │
│   ├── REST API: /api/telemetry, /api/event, /api/door, /api/settings              │
│   ├── Anti-Replay Engine: Deduplicates (device_id, seq), returns 409 Conflict     │
│   ├── Device Watchdog: Marks DEVICE_OFFLINE after 30s silence, DEVICE_ONLINE      │
│   │   on resume, with automatic client reconnection                               │
│   └── Bearer Token & Device Token Authentication                                  │
│                                                                                   │
│  Database (Supabase PostgreSQL):                                                  │
│   └── Tables: telemetry, events, door_events, attendance, settings, alerts        │
│                                                                                   │
│  Management Dashboard (React + Tailwind CSS):                                     │
│   └── Real-time telemetry cards, live camera stream, door toggle, security logs   │
+───────────────────────────────────────────────────────────────────────────────────+
```

---

## 2. Hardware Wiring & Pin Configuration

| Component | ESP32-S3-CAM Pin | Details |
|---|---|---|
| **DHT11 Sensor** | `GPIO 15` | Single-bus data line with internal pull-up |
| **RC522 RFID (SPI)** | `SCK: 14`, `MISO: 12`, `MOSI: 13`, `SDA/SS: 21`, `RST: 2` | SPI Bus, 3.3V VCC |
| **Flame Sensor (DO)** | `GPIO 47` | Active-LOW digital trigger (2s debounce) |
| **Simulated Door** | `GPIO 48` | Active-HIGH relay / solenoid driver |
| **Buzzer** | `GPIO 38` | Active-HIGH piezoelectric buzzer |
| **Panic Touch Pad** | `GPIO 4` & `GPIO 1` | Capacitive touch sensor (simultaneous baseline calibration) |
| **BOOT Button** | `GPIO 0` | Factory reset (hold 5s) / OTA trigger (hold 3s) |
| **Onboard RGB LED** | `GPIO 48` | WS2812 / NeoPixel visual diagnostic indicator |

---

## 3. Firmware Build & Flash Instructions

### Prerequisites & Board Setup
1. Install **Arduino IDE 2.x** or later.
2. In Arduino IDE Settings, add the ESP32 board package URL:
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
3. Install **esp32 by Espressif Systems** (version 3.0.x or 3.1.x).
4. Select Board: **ESP32S3 Dev Module**.
5. Configure Board Settings in the **Tools** menu:
   - **USB CDC On Boot:** `Enabled` (Crucial for Serial Monitor logs)
   - **CPU Frequency:** `240MHz (WiFi)`
   - **Core Debug Level:** `None`
   - **Flash Mode:** `QIO 80MHz`
   - **Flash Size:** `8MB (64Mb)` or `4MB`
   - **Partition Scheme:** `Huge APP (3MB No OTA / 1MB SPIFFS)` or `Default 4MB with LittleFS`
   - **PSRAM:** `OPI PSRAM`

### Required Arduino Libraries
Install the following libraries via the Arduino Library Manager (`Ctrl + Shift + I`):
- `SmartProv` (v2.1.3+)
- `MFRC522` (v1.4.11+)
- `DHT sensor library` by Adafruit
- `ArduinoJson` (v7.x)
- `LittleFS` (built-in with ESP32 core)

### Compiling & Flashing
1. Open `firmware/main/main.ino` in Arduino IDE.
2. Connect the ESP32-S3-CAM via USB-C.
3. Select the active COM port.
4. Click **Upload** (`Ctrl + U`).
5. Open Serial Monitor (`Ctrl + Shift + M`) set to **115200 baud**.

---

## 4. Cloud Platform & Backend Setup

### Prerequisites
- Python 3.10 or higher
- A Supabase project (PostgreSQL)

### Backend Installation & Startup
```bash
# 1. Navigate to the backend directory
cd backend

# 2. Create and activate a Python virtual environment
python -m venv venv
venv\Scripts\activate      # Windows (PowerShell / CMD)
# source venv/bin/activate # Linux / macOS

# 3. Install dependencies
pip install -r requirements.txt

# 4. Configure environment variables
cp .env.example .env
# Edit .env and supply your Supabase credentials:
# SUPABASE_URL=https://your-project.supabase.co
# SUPABASE_ANON_KEY=your-anon-key

# 5. Run the FastAPI server
uvicorn main:app --host 0.0.0.0 --port 8000 --reload
```

### Database Initialization
Execute the SQL DDL statements found in `backend/supabase/schema.sql` inside your **Supabase SQL Editor** to initialize the required tables (`telemetry`, `events`, `sensor_logs`, `attendance`, `door_events`, `settings`, `alerts`).

---

## 5. Level 2: Wi-Fi Setup, Captive Portal & Secure Telemetry

### Initial Access Point & Captive Portal Provisioning
1. If no network credentials exist in flash (or upon issuing a reset), the device starts SoftAP **`SmartClass-T10`**.
2. An 8-character random alphanumeric password is generated and printed on the Serial Monitor:
   ```
   [NET] Starting AP SmartClass-T10, pass=XXXXXXXX at 192.168.4.1
   ```
3. The onboard LED pulses with a **Blue 2 Hz heartbeat**.
4. Connect a laptop or smartphone to Wi-Fi `SmartClass-T10` using the printed 8-character password.
5. Open your browser and navigate to `http://192.168.4.1` (or follow the Captive Portal redirect prompt).
6. The setup page:
   - Scans and lists all nearby 2.4 GHz Wi-Fi SSIDs with signal strength (RSSI).
   - Provides input fields for **Wi-Fi Password**, **Room ID** (`room_id`), **Device Token** (`device_token`), and **Cloud URL** (`cloud_url`).
   - Persists all configuration into non-volatile flash storage (**NVS**).
7. If an incorrect Wi-Fi password is submitted, the device returns to setup mode within 15 seconds, displays an error alert directly on the setup web page, and logs `[NET] Setup error: ...` to the Serial Monitor.

### Local Status Page & mDNS
Once Wi-Fi connection is established:
- The device responds on mDNS at **`http://smartclass-t10.local`** and at its assigned local IP address.
- The web page is served directly from the board's flash file system (**LittleFS**).
- The page updates itself every 2 seconds via live asynchronous polling (`/status.json`) and `<meta http-equiv="refresh" content="2">`.
- Displays all 12+ required parameters:
  `Room ID`, `Local Time (Asia/Dhaka)`, `Connection State (LIVE / OFFLINE)`, `Temperature`, `Humidity`, `Heat Index`, `Heat Trend`, `Data Quality`, `Flame Status`, `Door State`, `Last Card UID`, `Wi-Fi RSSI`, `Chip Temperature`, `Firmware Version`, and `System Uptime`.

### Telemetry & Anti-Replay Protection
- Telemetry is transmitted every 5 ± 1 seconds over HTTPS (`/api/telemetry`).
- Each payload carries a strictly monotonically increasing sequence counter (`seq`) persisted across reboots via NVS, together with internet NTP timestamp (`ts`).
- Dual-format JSON schema is transmitted (both `camelCase` and `snake_case` keys) for universal platform compatibility:
  ```json
  {
    "device_id": "T10-D1",
    "deviceId": "T10-D1",
    "room_id": "ROOM-01",
    "roomId": "ROOM-01",
    "seq": 42,
    "ts": 1731512400,
    "uptime_s": 210,
    "uptimeS": 210,
    "temp_c": 28.5,
    "tempC": 28.5,
    "rh": 62.0,
    "heat_index_c": 29.8,
    "heatIndexC": 29.8,
    "flame": false,
    "door": "LOCKED",
    "status": "Safe",
    "heat_trend": "STABLE",
    "heatTrend": "STABLE",
    "rssi": -55,
    "heap_kb": 184,
    "heapKb": 184,
    "chip_temp_c": 46.2,
    "chipTempC": 46.2,
    "fw": "1.1.0",
    "quality": "good"
  }
  ```
- **Replay Attack Rejection:** The backend validates `(device_id, seq)` against stored records; duplicate sequence numbers are immediately rejected with **HTTP 409 Conflict**.
- **Offline / Online State Detection:** An automated background monitor inspects device activity; silence exceeding 30 seconds records `DEVICE_OFFLINE` in the database. Upon resumption, `DEVICE_ONLINE` is logged and normal telemetry continues without device reset.
- **Fail-safe Offline Queue:** If network drops, events and telemetry are automatically buffered in LittleFS (`offlineQueue`) and dispatched (`offlineFlush`) as soon as Wi-Fi restores.

### Factory Reset & Credential Clearing
- **Serial Command:** Enter `reset` into the Serial Monitor.
- **Hardware Button:** Hold the **BOOT** button (GPIO 0) for 5 seconds.
- Result: Wi-Fi credentials and temporary tokens are erased from flash, and the device restarts into provisioning setup mode with a fresh 8-character password.

---

## 6. Secret Safety & Repository Integrity

This repository strictly complies with the competition's zero-secret policy:
- **No hardcoded secrets:** No passwords, private API keys, JWT secrets, or device tokens are committed anywhere in the codebase or git history.
- **Environment Driven:** Backend credentials reside solely in untracked `.env` files.
- **NVS Dynamic Storage:** All Wi-Fi passwords, room identifiers, and device tokens are provisioned at runtime via the captive portal and stored in encrypted microcontroller NVS partitions.

---

## 7. Git Tags & Release History

| Tag | Phase / Level Description |
|---|---|
| `level-01` | Level 1: Dual-core FreeRTOS, 2 Hz RGB LED, 1s `[STATUS]` log, Watchdog reboot on `hang`, sensor error handling, single `[CARD]` tap debounce. |
| `level-02` | Level 2: SoftAP `SmartClass-T10`, 8-char random password, Captive Portal at `192.168.4.1`, LittleFS status page at `http://smartclass-t10.local`, HTTPS telemetry with NTP and monotonic `seq`, 409 replay rejection, 30s offline tracking. |
