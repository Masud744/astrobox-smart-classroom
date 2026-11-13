# 📅 Competition Day Guide — FastRobox T07
**Hall-এ ঢোকার পর থেকে শেষ পর্যন্ত**

> [!CAUTION]
> Max 4 judging calls. প্রতিটা level-এ 2 attempts. দুটো fail = বাকি সব levels বন্ধ।
> **Call দেওয়ার আগে VERIFICATION-CHECKLIST.md-এ সব tick হতে হবে।**

---

## ⏰ Timeline (7 Hours)

| Time | কাজ |
|------|-----|
| **H+0:00** | Seat নাও, hardware unbox, laptop setup |
| **H+0:15** | Arduino IDE open, firmware upload করো |
| **H+0:30** | SmartProv test: Phone-এ SmartClass-T07 দেখা যাচ্ছে? |
| **H+0:45** | Backend URL update করো (competition WiFi IP), dashboard build |
| **H+1:00** | Level 1 test: watchdog, serial, LED heartbeat |
| **H+1:30** | Level 2 test: WiFi, mDNS, telemetry |
| **H+2:00** | Level 3 test: heat, fire, panic |
| **H+2:15** | VERIFICATION-CHECKLIST → L1+L2+L3 সব tick? |
| **H+2:30** | 🔔 **CALL 1** → Level 1 + 2 + 3 |
| **H+3:00** | Level 4: auth roles, Telegram |
| **H+3:30** | Level 5: OTA, settings |
| **H+3:45** | VERIFICATION-CHECKLIST → L4+L5 সব tick? |
| **H+4:00** | 🔔 **CALL 2** → Level 4 + 5 |
| **H+4:30** | Level 6: weather, AI |
| **H+5:30** | Level 7: face check-in |
| **H+5:50** | VERIFICATION-CHECKLIST → L6+L7 সব tick? |
| **H+6:00** | 🔔 **CALL 3** → Level 6 + 7 |
| **H+6:30** | Level 8: anti-spoof |
| **H+6:50** | VERIFICATION-CHECKLIST → L8 tick? |
| **H+7:00** | 🔔 **CALL 4** → Level 8 |

---

## 🔔 Call Script (মুখে বলবে)

**Call 1 শুরুতে:**
> "আমরা Team T07, FastRobox 1.0। আমরা আজকে Level 1, 2, এবং 3-এর জন্য judging call দিচ্ছি।"

**প্রতিটা feature দেখানোর সময়:**
> "এখন আমি [feature] দেখাচ্ছি। [action করো] → [result point করো]।"

**কিছু কাজ না করলে:**
> "এই feature-টা আমরা চেক করে আসি, একটু সময় দিন।"
> (Call reschedule করো, panic করো না)

---

## 🚨 Emergency Fixes (যদি কিছু কাজ না করে)

| সমস্যা | Quick fix |
|---------|-----------|
| Backend unreachable | `curl <url>/` check করো, Railway-এ restart করো |
| Supabase 400/500 error | Browser DevTools → Network → Response body দেখো |
| ESP32 crash loop | `hang` command দেওয়া হয়েছিল? Serial Monitor দেখো |
| WiFi না দেখালে | Arduino IDE-তে AP mode manually trigger করো |
| Dashboard blank | `VITE_API_URL` correct আছে? build করা হয়েছে? |
| LED কাজ করছে না | `PIN_RGB_LED` = 48, `RGB_BUILTIN` check করো |
| DHT11 NaN | Wire connection + pull-up resistor check করো |
| RFID not reading | SCK=39, MISO=40, MOSI=41, SDA=42, RST=2 check করো |

---

## 📋 Hardware Setup Checklist

```
[ ] ESP32-S3-CAM power LED on
[ ] USB to Serial → Arduino IDE-এ correct port
[ ] RC522 wired: SCK=39, MISO=40, MOSI=41, SDA=42, RST=2
[ ] DHT11: data=GPIO21, 10kΩ pull-up to 3.3V
[ ] Flame sensor: GPIO47, active LOW
[ ] Buzzer: GPIO38, active HIGH
[ ] Touch pad: GPIO1 (jumper wire)
[ ] RGB LED: GPIO48 (onboard)
[ ] BOOT button: GPIO0 (onboard)
```

---

## 🎥 Video Recording Tips

- Stopwatch always visible (phone stopwatch বা screen-এ)
- Serial Monitor দেখা যাচ্ছে (font size বড় করো)
- Screen + hardware একসাথে frame-এ
- Max 10 min, 720p+
- File name: `T07_L01_video.mp4`

---

## 📝 Write-up Structure (প্রতিটা level-এ)

```
1. Level number + title
2. What we implemented (bullet list)
3. How it works (1-2 paragraph)
4. Demo steps performed
5. Any deviations from spec (if any)
6. Time taken
```

---

## 🏆 Scoring Target

| Call | Levels | Marks | Cumulative |
|------|--------|-------|------------|
| Call 1 | L1+L2+L3 | 130 | 130 |
| Call 2 | L4+L5 | 70 | 200 |
| Call 3 | L6+L7 | 90 | 290 |
| Call 4 | L8 | 30 | 320 |

**Minimum target: Call 1 clear = 130 marks (pass level minimum)**
