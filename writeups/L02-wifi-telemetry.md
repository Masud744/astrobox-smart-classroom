# Level 2 — WiFi Provisioning & Telemetry Writeup
**Team: T07 | FastRobox 1.0 | BUBT Techathon Grand Finale**

---

## 1. Level Information

| Field | Value |
|-------|-------|
| **Level** | Level 2 |
| **Title** | WiFi Setup, mDNS & Cloud Telemetry |
| **Marks** | __ / 45 |
| **Call Number** | Call 1 |
| **Pass Time** | __ : __ |
| **Time Taken** | __ minutes |

---

## 2. What We Built

Level 2 required WiFi provisioning via captive portal (SmartProv), mDNS discovery, LittleFS migration, and structured telemetry with replay-rejection. The device broadcasts a `SmartClass-T07` AP with a randomly generated 8-character password, allows room configuration via browser, then connects to the classroom WiFi and starts sending telemetry to the cloud with an incrementing `seq` counter.

---

## 3. Features Implemented

| Feature | Status | Notes |
|---------|--------|-------|
| AP name: `SmartClass-T07` | ✅ Done | Team ID in name |
| Random 8-char password (NVS) | ✅ Done | Generated once, persists across restarts |
| Password printed to Serial | ✅ Done | `[NET] Setup AP: SmartClass-T07 | password: XXXXXXXX` |
| SmartProv captive portal | ✅ Done | Nearby WiFi scan + form |
| `device_token` field via SmartProv | ✅ Done | Stored in NVS |
| mDNS: `smartclass-t07.local` | ✅ Done | `ESPmDNS.h` |
| SPIFFS → LittleFS migration | ✅ Done | Full migration |
| `seq` counter in NVS | ✅ Done | Never reused after restart |
| Telemetry exact field names | ✅ Done | 17 fields per spec |
| `[NET]` prefix logs | ✅ Done | Connect/disconnect/reconnect |
| WiFi auto-reconnect (no restart) | ✅ Done | Exponential backoff |
| `/api/telemetry` with 409 on replay | ✅ Done | `UNIQUE(device_id, seq)` |
| Device offline tracking (30s) | ✅ Done | Backend background task |

---

## 4. How It Works

### Firmware

On first boot, the device generates a random 8-character alphanumeric password and saves it to NVS (Preferences). This password never changes unless NVS is wiped. The device starts a `SmartClass-T07` access point, and a SmartProv captive portal lets the user select a WiFi network, enter credentials, room ID, and device token — all stored in NVS, never hardcoded.

After connecting, `ESPmDNS.h` registers `smartclass-t07.local` so the device is accessible by hostname. We migrated all SPIFFS calls to LittleFS for better wear leveling and reliability. Each telemetry packet includes a `seq` counter that increments in NVS on every send, ensuring no sequence number is ever reused even after a restart.

### Backend

The `/api/telemetry` endpoint checks `(device_id, seq)` uniqueness before inserting. A duplicate returns 409 Conflict. A background async task runs every 30 seconds and checks for devices whose last telemetry is older than 30 seconds, posting a `DEVICE_OFFLINE` event.

---

## 5. Demo Steps Performed

1. Factory reset → `SmartClass-T07` appeared on phone WiFi list
2. Showed 8-char password in Serial: `[NET] Setup AP: SmartClass-T07 | password: XXXXXXXX`
3. Opened captive portal → showed nearby networks list
4. Configured WiFi → device connected → `[NET] Connected | IP: 192.168.x.x`
5. Browser opened `http://smartclass-t07.local` → local status page loaded
6. Supabase telemetry table → showed rows arriving every 5s, seq incrementing
7. Sent same seq again via curl → got `409 Conflict`
8. Unplugged WiFi router for 20s → auto-reconnected → uptime continued (not reset)

---

## 6. Evidence

| Item | File Name |
|------|-----------|
| Video | `T07_L02_video.mp4` |
| Writeup PDF | `T07_L02_writeup.pdf` |
| Git Tag | `level-02` |
| Commit | [paste commit hash here] |

---

## 7. Any Deviations from Spec

- None

---

## 8. Lessons Learned

[Fill after competition]
