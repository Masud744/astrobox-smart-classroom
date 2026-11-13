# Level 1 — Dual-Core, Health & Watchdog Writeup
**Team: T07 | FastRobox 1.0 | BUBT Techathon Grand Finale**

---

## 1. Level Information

| Field | Value |
|-------|-------|
| **Level** | Level 1 |
| **Title** | Dual-Core Architecture, Health Monitoring & Watchdog |
| **Marks** | __ / 35 |
| **Call Number** | Call 1 |
| **Pass Time** | __ : __ |
| **Time Taken** | __ minutes |

---

## 2. What We Built

Level 1 required a robust firmware foundation with dual-core FreeRTOS task management, a Task Watchdog Timer, real-time health monitoring via Serial, and sensor error detection. We implemented all required features on the ESP32-S3-CAM running at 240 MHz with OPI PSRAM enabled.

---

## 3. Features Implemented

| Feature | Status | Notes |
|---------|--------|-------|
| `[EVENT] BOOT reason=` on startup | ✅ Done | Using `esp_reset_reason()` |
| Task Watchdog Timer (10s timeout) | ✅ Done | `esp_task_wdt.h`, panic on timeout |
| `[STATUS]` line every 1 second | ✅ Done | Exact format per spec |
| `help` serial command | ✅ Done | Lists all commands |
| `status` serial command | ✅ Done | Prints current status |
| `health` serial command | ✅ Done | chip temp, heap, PSRAM |
| `tasks` serial command | ✅ Done | `vTaskList()` output |
| `ver` serial command | ✅ Done | fw=1.0.0 chip=ESP32-S3 |
| `hang` serial command | ✅ Done | Tests watchdog (12s hang) |
| LED 2 Hz heartbeat | ✅ Done | FreeRTOS task, 250ms on/off |
| DHT11 timeout detection (12s) | ✅ Done | 3 consecutive NaN = error |
| RFID timeout detection (3s) | ✅ Done | Magenta LED + [ERROR] log |
| `[CARD]` log with 2s debounce | ✅ Done | Same card ignored for 2s |

---

## 4. How It Works

### Firmware

The firmware runs on ESP32-S3-CAM with FreeRTOS. Core 0 handles the main loop (sensors, cloud, serial commands) and Core 1 handles the camera stream. We added `esp_task_wdt.h` with a 10-second timeout — if the main loop hangs (e.g., from the `hang` command), the WDT fires and restarts the device, logging `[EVENT] BOOT reason=WATCHDOG` on the next boot.

The `[STATUS]` line is printed every 1 second using a non-blocking timer (`millis()` diff). It includes uptime, temperature, humidity, heat index (placeholder until Level 3), flame state, door state, room state, heat trend, WiFi RSSI, free heap, free PSRAM, chip temperature, and firmware version — all in one line exactly as specified.

The LED heartbeat runs in a dedicated FreeRTOS task that toggles the RGB LED every 250ms (2 Hz), reflecting the current room state color: green for SAFE, yellow for WARNING, red for DANGER, magenta for ERROR.

Sensor error detection tracks consecutive failed DHT11 reads (NaN values, 4s apart = 12s total) and RFID SPI timeout. On error, the LED turns magenta and an `[ERROR]` line is printed to Serial. The error clears automatically when the sensor recovers.

---

## 5. Demo Steps Performed

1. Powered on ESP32 → Serial showed `[EVENT] BOOT reason=POWER_ON`
2. Showed `[STATUS]` line printing every 1 second (counted with stopwatch)
3. Showed green LED 2 Hz heartbeat (visible blinking)
4. Typed `health` → showed chip temp, heap KB, PSRAM KB, uptime
5. Typed `tasks` → showed all running FreeRTOS tasks
6. Typed `ver` → showed `fw=1.0.0 chip=ESP32-S3`
7. Typed `hang` → 10 seconds later device restarted → `[EVENT] BOOT reason=WATCHDOG`
8. Unplugged DHT11 data wire → after 12s showed `[ERROR] SENSOR ERROR: DHT11` + magenta LED
9. Reconnected DHT11 → error cleared, green heartbeat resumed
10. Tapped RFID card → `[CARD] uid=XX:XX:XX:XX` appeared
11. Tapped same card within 2s → no duplicate log (debounce working)

---

## 6. Evidence

| Item | File Name |
|------|-----------|
| Video | `T07_L01_video.mp4` |
| Writeup PDF | `T07_L01_writeup.pdf` |
| Git Tag | `level-01` |
| Commit | [paste commit hash here] |

---

## 7. Any Deviations from Spec

- None

---

## 8. Lessons Learned

[Fill after competition]

