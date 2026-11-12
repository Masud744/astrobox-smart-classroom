# ✅ Verification Report — 2026-11-13

## Backend API — 10/10 Tests Passed ✅

| Endpoint | Method | Result |
|----------|--------|--------|
| `/` | GET | ✅ `{status: ok}` |
| `/api/settings` | GET | ✅ Returns default config |
| `/api/sensor` | POST | ✅ Data saved to Supabase |
| `/api/sensor/latest` | GET | ✅ Returns latest reading |
| `/api/attendance` | POST | ✅ RFID log saved |
| `/api/door` | POST | ✅ Door event + settings updated |
| `/api/alert` | POST | ✅ Alert saved |
| `/api/heartbeat` | POST | ✅ Device ping saved |
| `/api/settings` | PATCH | ✅ Remote config works |
| `/api/ota/latest` | GET | ✅ Returns `{update: false}` correctly |

## Files — 18/18 Present ✅

All project files, skills, firmware, dashboard, and config verified.

## Security — ✅

- `.env` NOT tracked in git ✅
- No hardcoded WiFi/secrets in firmware ✅
- `node_modules/` gitignored ✅

## Supabase — ✅

- Project: `fastrobox-smart-classroom`
- URL: `https://oxxfnnecwvslmkxqhfgb.supabase.co`
- Region: Singapore (ap-southeast-1)
- Tables: 7 created, RLS enabled
- Default settings seeded

---

# 📋 বাকি কাজ যা এখনই করা যাবে

## Priority 1 — এখনই করো

| কাজ | কেন গুরুত্বপূর্ণ |
|-----|----------------|
| **Supabase Realtime** dashboard-এ add করো | 5s polling এর বদলে instant update |
| **SPIFFS offline queue** firmware-এ add করো | WiFi গেলেও data হারাবে না |
| **OTA firmware code** (`ota.cpp`) লিখো | Level-এ চাইতে পারে |
| **Camera stream code** লিখো | Face recognition-এর আগে দরকার |
| **RFID NVS registration** system লিখো | Competition-এ card scan করলে role assign করতে হবে |
| **Backend deploy** (Railway) করো | Public HTTPS URL দরকার competition-এ |

## Priority 2 — Hardware পেলে করবে

| কাজ | কেন |
|-----|-----|
| Pin mapping verify করো | ESP32-S3-CAM-এর exact GPIO আলাদা হতে পারে |
| SmartProv captive portal test করো | WiFi provisioning সরাসরি test |
| RFID card UID read করো | তারপর NVS-এ register করবে |
| DHT11, Flame sensor test করো | Response time measure করবে |
| Face registration করবে | Team members-এর face দিয়ে |

## Priority 3 — Competition-এর আগের দিন

| কাজ | কেন |
|-----|-----|
| End-to-end full flow test করো | সব একসাথে |
| Write-up template pre-fill করো | শুধু data বসাতে হবে |
| Video recording setup test করো | 720p, stopwatch দেখা যাচ্ছে কিনা |
| Judging call script practice করো | "আমরা Level 1, 2, 3 জন্য call দিচ্ছি" |
| Checklist print করে রাখো | Competition-এ দেখে দেখে করবে |

---

# ⚠️ যা কখনোই করা যাবে না (আগেভাগে)

| কাজ | কারণ |
|-----|------|
| WiFi credentials hardcode | Judge অন-স্পট দেবে |
| RFID card UID hardcode | Card competition-এর দিন দেবে |
| Face training data save করা | শুধু competition-এর দিন, শেষে delete |
| Exact level challenges জানা | Main question paper competition-এর দিন |
| Response time limits hardcode | Question paper-এ দেওয়া হবে |
