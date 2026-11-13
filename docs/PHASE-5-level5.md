# Phase 5 — Level 5: OTA + Rollback (30 marks)
**Call 2**

---

## 📋 Task List

### Firmware — `firmware/main/ota.cpp`

- [ ] **Post-OTA validation**: নতুন firmware চালু হওয়ার পর server reach করতে হবে 60s-এর মধ্যে
  ```cpp
  // নতুন firmware-এর setup()-এ:
  unsigned long start = millis();
  bool reached = false;
  while (millis()-start < 60000) {
    if (pingServer()) { reached=true; break; }
    delay(1000);
  }
  if (!reached) {
    Serial.println("[OTA] Server unreachable, rolling back!");
    esp_ota_mark_app_invalid_rollback_and_reboot();
  }
  ```

- [ ] **OTA progress logs**:
  ```
  [OTA] Starting download: v1.1.0
  [OTA] Progress: 25%
  [OTA] Progress: 50%
  [OTA] Progress: 75%
  [OTA] Flash complete, validating...
  [OTA] Validation OK, rebooting...
  ```

- [ ] **3টা test .bin file** prepare:
  - `valid_v1.1.0.bin` → higher version, connects OK
  - `damaged.bin` → cut in half (corrupt)
  - `faulty_v1.2.0.bin` → compiles OK but wrong `VITE_API_URL` hardcoded

### Backend — `backend/main.py`

- [ ] **`PUT /api/settings`** (Admin only):
  - temp threshold: warning/danger sliders
  - Validation: 0–50°C, warning_temp < danger_temp
  - Change log in `settings_log` table

- [ ] **Settings push**: device পরের 10s poll-এ নতুন settings পাবে → `[EVENT] SETTINGS_APPLIED`

- [ ] **`/api/ota/upload`** validation:
  - Magic bytes check: ESP32 firmware magic byte `0xE9` at offset 0
  - File size > 10KB

- [ ] **OTA events** log: `OTA_DONE`, `OTA_REJECTED`, `OTA_ROLLBACK`

---

## 🧪 Verification Test

```
[ ] Admin → temp threshold change → device-এ 10s-এর মধ্যে apply + [EVENT] SETTINGS_APPLIED
[ ] Power cut → settings survived restart (NVS-এ ছিল)
[ ] valid_v1.1.0.bin upload → OTA → new version [STATUS]-এ দেখাচ্ছে
[ ] damaged.bin upload → rejected → device running (old version)
[ ] faulty_v1.2.0.bin → installs → cannot reach server → auto rollback 60s-এ → [OTA] rolling back
[ ] After rollback → [EVENT] BOOT reason=SOFTWARE (or WATCHDOG)
```

## ✅ Done হলে

```bash
git add -A
git commit -m "level-05: OTA rollback 60s, settings push, /api/settings with change log, bin validation"
git tag level-05
git push origin main --tags
```
