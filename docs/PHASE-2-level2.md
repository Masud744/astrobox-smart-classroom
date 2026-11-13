# Phase 2 — Level 2: WiFi Setup + Telemetry (45 marks)
**Call 1-এ judge করাবো**

---

## 📋 Task List

### Firmware — `firmware/main/wifi_prov.cpp`

- [ ] **AP name**: `SmartClass-T07` (Team ID confirm করো)
- [ ] **8-char random password**: boot-এ generate করো, NVS-এ save করো (restart-এ same password)
  ```cpp
  Preferences prefs; prefs.begin("wifi");
  String pass = prefs.getString("appass", "");
  if (pass == "") {
    const char c[] = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghjkmnpqrstuvwxyz23456789";
    for(int i=0;i<8;i++) pass += c[random(sizeof(c)-1)];
    prefs.putString("appass", pass);
  }
  prefs.end();
  Serial.printf("[NET] Setup AP: SmartClass-T07 | password: %s\n", pass.c_str());
  ```
- [ ] WiFi connect হওয়ার পর → `[NET] Connected | IP: 192.168.1.x`

- [ ] **mDNS**: `http://smartclass-t07.local`
  ```cpp
  #include <ESPmDNS.h>
  MDNS.begin("smartclass-t07");
  Serial.println("[NET] mDNS started: http://smartclass-t07.local");
  ```

- [ ] **WiFi reconnect** (without restart): `WiFi.onEvent()` দিয়ে handle করো
  - disconnect → exponential backoff retry
  - reconnect → `[NET] Reconnected`, then `offlineFlush()`

### Firmware — সব ফাইলে SPIFFS → LittleFS

- [ ] `#include <SPIFFS.h>` → `#include <LittleFS.h>`
- [ ] `SPIFFS.begin(true)` → `LittleFS.begin(true)`
- [ ] সব `SPIFFS.` → `LittleFS.` replace করো
- [ ] Arduino IDE-তে LittleFS library install আছে কিনা confirm

### Firmware — `firmware/main/cloud.cpp`

- [ ] **seq counter** NVS-এ persist করো (restart-এ never reuse)
  ```cpp
  Preferences p; p.begin("cloud",false);
  uint32_t seq = p.getUInt("seq",0)+1;
  p.putUInt("seq",seq); p.end();
  ```
- [ ] **device_token** SmartProv field থেকে নাও (NVS-এ save করো)

- [ ] **Telemetry JSON exact field names** (paper অনুযায়ী):
  ```json
  {
    "device_id": "T07-D1",
    "room_id": "ROOM-01",
    "seq": 42,
    "ts": 1699876543,
    "uptime_s": 742,
    "temp_c": 29.0,
    "rh": 64.0,
    "heat_index_c": null,
    "flame": false,
    "door": "LOCKED",
    "status": "SAFE",
    "heat_trend": null,
    "rssi": -61,
    "heap_kb": 182,
    "chip_temp_c": 41.5,
    "fw": "1.0.0",
    "quality": "good"
  }
  ```

- [ ] `[NET]` prefix সব network-related log-এ

### Backend — `backend/main.py`

- [ ] **`POST /api/telemetry`** endpoint:
  - `(device_id, seq)` duplicate check → 409 Conflict
  - Save to `telemetry` table
- [ ] **`POST /api/event`** endpoint (save to `events` table)
- [ ] **Device offline tracking**: background task প্রতি 30s → last telemetry > 30s → `events`-এ `DEVICE_OFFLINE` insert

### Supabase

- [ ] `migration_v2.sql` চালাও (telemetry table + events table)

---

## 🧪 Verification Test

```
[ ] Phone-এ SmartClass-T07 WiFi দেখাচ্ছে
[ ] Setup page-এ nearby WiFi networks list আসছে
[ ] Connect করার পর Serial-এ IP + mDNS address print হচ্ছে
[ ] Browser-এ http://smartclass-t07.local → local status page open হচ্ছে
[ ] Supabase telemetry table-এ প্রতি 5s নতুন row + seq বাড়ছে (1,2,3...)
[ ] same seq আবার POST করলে → 409 Conflict
[ ] WiFi router off 20s → auto reconnect → uptime চলতে থাকে (reset হয় না)
[ ] [NET] prefix log দেখাচ্ছে connect/disconnect-এ
```

## ✅ Done হলে

```bash
git add -A
git commit -m "level-02: AP SmartClass-T07, mDNS, LittleFS, seq NVS, telemetry exact fields, /api/telemetry replay reject"
git tag level-02
git push origin main --tags
```

