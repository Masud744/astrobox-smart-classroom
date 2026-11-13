# Level 5 — OTA Update & Rollback Writeup
**Team: T07 | FastRobox 1.0 | BUBT Techathon Grand Finale**

---

## 1. Level Information

| Field | Value |
|-------|-------|
| **Level** | Level 5 |
| **Title** | OTA Firmware Update with Auto-Rollback |
| **Marks** | __ / 30 |
| **Call Number** | Call 2 |
| **Pass Time** | __ : __ |
| **Time Taken** | __ minutes |

---

## 2. What We Built

Level 5 required OTA firmware updates with post-flash validation — if the new firmware cannot reach the backend within 60 seconds, it automatically rolls back to the previous version. Additionally, remote settings (temperature thresholds) can be changed from the Admin dashboard and the device applies them within 10 seconds.

---

## 3. Features Implemented

| Feature | Status | Notes |
|---------|--------|-------|
| OTA download + flash | ✅ Done | Using `HTTPUpdate` |
| Post-OTA server validation (60s) | ✅ Done | Auto-rollback if fails |
| `[OTA]` progress logs | ✅ Done | 25%/50%/75%/done |
| Auto-rollback on server unreachable | ✅ Done | `esp_ota_mark_app_invalid_rollback_and_reboot()` |
| OTA_DONE/OTA_REJECTED/OTA_ROLLBACK events | ✅ Done | Logged to events table |
| Damaged .bin rejected | ✅ Done | Magic byte `0xE9` check |
| `/api/settings` PUT (Admin) | ✅ Done | |
| Validation: 0–50°C, warning < danger | ✅ Done | |
| Settings pushed to device in 10s | ✅ Done | Settings poll interval |
| `[EVENT] SETTINGS_APPLIED` | ✅ Done | Logged on device |
| Settings change log | ✅ Done | `settings_log` table |
| Settings survive restart | ✅ Done | Stored in NVS |

---

## 4. How It Works

### Firmware

After flashing new firmware and rebooting, the new firmware attempts to reach the backend within 60 seconds. If successful, it marks itself as valid (`esp_ota_mark_app_valid_cancel_rollback()`). If the server is unreachable after 60 seconds, it calls `esp_ota_mark_app_invalid_rollback_and_reboot()`, which reverts to the previous partition and boots the old firmware. The rollback is fully automatic — no user intervention needed.

### Backend

The `/api/settings` PUT endpoint validates that `warning_temp < danger_temp` and both are within 0–50°C range. Every change is logged to the `settings_log` table with the user ID, field name, old value, new value, and timestamp. The device receives updated settings on its next 10-second poll cycle and logs `[EVENT] SETTINGS_APPLIED` to Serial.

---

## 5. Demo Steps Performed

1. Admin changed danger threshold from 40°C to 38°C → within 10s device logged `[EVENT] SETTINGS_APPLIED`
2. Power cycled device → new threshold still applied (NVS survived)
3. Uploaded `valid_v1.1.0.bin` → OTA ran → new version appeared in `[STATUS]`
4. Uploaded `damaged.bin` → rejected immediately (magic byte check failed)
5. Uploaded `faulty_v1.2.0.bin` → installed → device rebooted → server unreachable → after 60s `[OTA] rolling back!` → old firmware restored
6. After rollback `[STATUS]` showed original version number

---

## 6. Evidence

| Item | File Name |
|------|-----------|
| Video | `T07_L05_video.mp4` |
| Writeup PDF | `T07_L05_writeup.pdf` |
| Git Tag | `level-05` |
| Commit | [paste commit hash here] |

---

## 7. Any Deviations from Spec

- None

---

## 8. Lessons Learned

[Fill after competition]

