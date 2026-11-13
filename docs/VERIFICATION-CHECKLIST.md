# ✅ FastRobox — Master Verification Checklist
**Call দেওয়ার আগে এই file-এ সব tick হতে হবে**

> [!IMPORTANT]
> প্রতিটা phase শেষে এই file-এ এসে সব checkbox tick দাও। কোনো unchecked item থাকলে call দিও না।

---

## 🔴 Phase 0 — Pre-requisites

```
[ ] supabase.js-এ কোনো hardcoded key নেই
    → grep -r "eyJhbGci" dashboard/src/ → empty হতে হবে
[ ] dashboard/.env আছে (actual keys দিয়ে)
[ ] dashboard/.env.example আছে (placeholder দিয়ে)
[ ] App.jsx door trigger = 'dashboard' (not 'dashboard_teacher')
[ ] alerts table-এ 'unknown_card' আছে (Supabase-এ check করো)
[ ] telemetry table তৈরি হয়েছে (Supabase-এ check করো)
[ ] events table তৈরি হয়েছে (Supabase-এ check করো)
[ ] backend publicly deployed (curl করে test করো)
[ ] git status → clean
[ ] git log → কোনো secret নেই
```

---

## 🟠 Phase 1 — Level 1 (35 marks)
**Call 1-এর আগে এই সব হতে হবে**

### Firmware Test (hardware-এ করো):
```
[ ] Serial 115200 → [EVENT] BOOT reason=POWER_ON দেখাচ্ছে
[ ] প্রতি 1s → [STATUS] line print হচ্ছে
[ ] [STATUS] format exact:
    [STATUS] up=Xs T=X.XC RH=X% HI=- flame=X door=LOCKED state=SAFE trend=- ...
[ ] LED 2 Hz green heartbeat চোখে দেখা যাচ্ছে (গুনে দেখো: 2 blink/sec)
[ ] "help" command → সব command list দেখাচ্ছে
[ ] "health" command → chip temp, heap, PSRAM দেখাচ্ছে
[ ] "tasks" command → running tasks list দেখাচ্ছে
[ ] "ver" command → fw=1.0.0 chip=ESP32-S3
[ ] "hang" command → 10s পরে restart → [EVENT] BOOT reason=WATCHDOG
[ ] DHT11 wire unplug → 12s পরে [ERROR] SENSOR ERROR: DHT11 + LED magenta
[ ] DHT11 reconnect → error clear, LED green ফেরে
[ ] RFID tap → [CARD] uid=... log দেখাচ্ছে
[ ] Same card 2s-এর মধ্যে দুবার → শুধু একবার log
```

### Git:
```
[ ] git tag level-01 আছে
[ ] git push origin main --tags done
```

---

## 🟠 Phase 2 — Level 2 (45 marks)
**Call 1-এর আগে এই সব হতে হবে**

```
[ ] Phone-এ SmartClass-T07 WiFi দেখাচ্ছে
[ ] Setup page → nearby WiFi networks দেখাচ্ছে
[ ] Connect করার পর Serial-এ: [NET] Connected | IP: xxx.xxx.xxx.xxx
[ ] mDNS: Browser-এ http://smartclass-t07.local → local page open
[ ] Supabase telemetry table → প্রতি 5s নতুন row (seq: 1,2,3...)
[ ] seq duplicate POST → 409 Conflict
[ ] telemetry JSON-এ সব exact field আছে:
    device_id, room_id, seq, ts, uptime_s, temp_c, rh,
    heat_index_c, flame, door, status, heat_trend,
    rssi, heap_kb, chip_temp_c, fw, quality
[ ] WiFi cut 20s → auto reconnect → uptime চলতে থাকে
[ ] [NET] prefix log দেখাচ্ছে
[ ] SPIFFS কোথাও নেই (grep SPIFFS → empty)
```

### Git:
```
[ ] git tag level-02 আছে
[ ] git push origin main --tags done
```

---

## 🟡 Phase 3 — Level 3 (50 marks)
**Call 1-এর আগে এই সব হতে হবে**

```
[ ] 32°C → 3s hold পরে WARNING state (yellow LED)
[ ] 40°C → 3s hold পরে DANGER state (red LED + buzzer/2s)
[ ] Cool to 38°C → Warning (not Safe directly — hysteresis!)
[ ] Cool to 30°C AND RH ≤80% → Safe (green LED)
[ ] Flame 1s → nothing
[ ] Flame 2s continuous → FIRE: red fast + door opens + [EVENT] FIRE
[ ] Power cut during FIRE → restart → door still open (NVS!)
[ ] Flame off → Teacher card → buzzer silences
[ ] Flame on + Teacher card → "CANNOT SILENCE – FLAME PRESENT"
[ ] Touch <1s → nothing
[ ] Touch 2s → PANIC: red-blue alternating + cloudPostAlert("panic")
[ ] [STATUS] HI= real value (not -)
[ ] [STATUS] trend= real value (RISING/STABLE/FALLING)
[ ] quality field: "bad" when sensor out of range
```

### Git:
```
[ ] git tag level-03 আছে
[ ] git push origin main --tags done
```

---

## 🟢 Phase 4 — Level 4 (40 marks)
**Call 2-এর আগে এই সব হতে হবে**

```
[ ] 4 roles (admin/teacher/student/guardian) সব login করতে পারছে
[ ] Student → Teacher page → "Access Denied"
[ ] Guardian → শুধু linked children-এর data দেখছে
[ ] Dashboard → 3s-এর মধ্যে নতুন data দেখায়
[ ] PANIC → red banner 3s-এ + Telegram message 10s-এর মধ্যে
[ ] 5 wrong passwords → 30s lock → correct pass rejected → 30s পরে ok
[ ] Bangla toggle → 2+ page-এ সব label বাংলায়
[ ] Admin → login history page → সব attempt দেখাচ্ছে
[ ] Device health panel → uptime, RSSI, heap, chip temp, fw version
[ ] FIRE → red banner সব logged-in user-এ
[ ] Reset fire button → Teacher/Admin only → works
```

### Git:
```
[ ] git tag level-04 আছে
[ ] git push origin main --tags done
```

---

## 🟢 Phase 5 — Level 5 (30 marks)
**Call 2-এর আগে এই সব হতে হবে**

```
[ ] Admin → temp threshold change → device-এ 10s-এ apply
[ ] [EVENT] SETTINGS_APPLIED Serial-এ দেখাচ্ছে
[ ] Power cut → settings survived
[ ] valid_v1.1.0.bin → OTA → new version [STATUS]-এ দেখাচ্ছে
[ ] damaged.bin → rejected → old version চলছে
[ ] faulty_v1.2.0.bin → installs → server unreachable → auto rollback 60s-এ
[ ] After rollback → [OTA] Server unreachable, rolling back! Serial-এ
[ ] OTA_DONE / OTA_REJECTED / OTA_ROLLBACK → events table-এ আছে
```

### Git:
```
[ ] git tag level-05 আছে
[ ] git push origin main --tags done
```

---

## 🔵 Phase 6 — Level 6 (40 marks)
**Call 3-এর আগে এই সব হতে হবে**

```
[ ] Weather panel → current data দেখাচ্ছে (not empty)
[ ] TEST notice via API → purple LED + 3 beeps within 10s
[ ] Weather service down → "Data stale (Xm ago)" (not error)
[ ] Chart → last 24h line graph
[ ] Event filter FIRE → CSV = page data exactly
[ ] AI: "highest temp today?" → correct DB answer
[ ] AI: "data for yesterday?" → "No data available"
[ ] AI: Bangla question → works
```

### Git:
```
[ ] git tag level-06 আছে
[ ] git push origin main --tags done
```

---

## 🔵 Phase 7 — Level 7 (50 marks)
**Call 3-এর আগে এই সব হতে হবে**

```
[ ] Card + face → GRANTED → door 5s green → auto-lock
[ ] No face 3s → NO_FACE denial (2 beeps)
[ ] Wrong class → NOT IN THIS CLASS
[ ] No class running → NO CLASS NOW
[ ] Unknown card → UNKNOWN CARD
[ ] Double check-in → 1 attendance record only
[ ] 30+ min no check-in → Guardian Telegram: "Absent"
[ ] Attendance table: Present/Late/Absent visible
[ ] No actual photos stored (only embeddings)
```

### Git:
```
[ ] git tag level-07 আছে
[ ] git push origin main --tags done
```

---

## ⚫ Phase 8 — Level 8 (30 marks)
**Call 4-এর আগে এই সব হতে হবে**

```
[ ] 3 mismatches in 2 min → card blocked 5 min
[ ] Admin unblocks → works
[ ] Photo on phone → denied (liveness)
[ ] Two teachers ack simultaneously → 1 ack in DB
[ ] Student delete → all data gone + audit log
```

### Git:
```
[ ] git tag level-08 আছে
[ ] git push origin main --tags done
```

---

## 🔐 Security Final Check (প্রতিটা push-এর আগে)

```
[ ] grep -r "eyJhbGci" dashboard/src/ → empty
[ ] grep -r "supabase.co" dashboard/src/ → empty (only import.meta.env)
[ ] git log --all --oneline | grep -i "key\|token\|pass" → empty
[ ] dashboard/.env → NOT in git (git status-এ দেখা যাচ্ছে না)
[ ] backend/.env → NOT in git
[ ] No hardcoded WiFi/room ID/timing in firmware
```

---

## 📸 Evidence Checklist (প্রতিটা level pass-এর পরে)

```
[ ] git tag level-NN + git push origin main --tags
[ ] Video: 720p+, max 10 min, stopwatch দেখা যাচ্ছে
[ ] Renamed: T07_L0N_video.mp4
[ ] Write-up: T07_L0N_writeup.pdf
[ ] Both uploaded to submission within 30 min
```
