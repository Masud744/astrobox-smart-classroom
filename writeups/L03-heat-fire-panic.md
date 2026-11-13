# Level 3 — Heat, Fire & Panic Writeup
**Team: T07 | FastRobox 1.0 | BUBT Techathon Grand Finale**

---

## 1. Level Information

| Field | Value |
|-------|-------|
| **Level** | Level 3 |
| **Title** | Thermal Monitoring, Fire Detection & Panic System |
| **Marks** | __ / 50 |
| **Call Number** | Call 1 |
| **Pass Time** | __ : __ |
| **Time Taken** | __ minutes |

---

## 2. What We Built

Level 3 required precise thermal state management with hysteresis, a Rothfusz heat index formula, heat trend tracking, 2-second flame debounce for fire detection (with NVS persistence across power cuts), and a 2-second touch hold for panic. All state changes require a 3-second hold timer to prevent false triggers.

---

## 3. Features Implemented

| Feature | Status | Notes |
|---------|--------|-------|
| Warning threshold: 32°C OR RH≥85% | ✅ Done | Fixed from old 35°C |
| Danger threshold: 40°C | ✅ Done | |
| 3-second hold timer | ✅ Done | Before any state change |
| Hysteresis: Danger→Warning at 38°C | ✅ Done | |
| Hysteresis: Warning→Safe at 30°C+RH≤80% | ✅ Done | |
| Heat index (Rothfusz equation) | ✅ Done | `HI=` in STATUS line |
| Heat trend (last 10 readings) | ✅ Done | RISING/STABLE/FALLING |
| `quality` field (bad if out of range) | ✅ Done | 0–50°C, 20–90% RH |
| Flame 2s continuous debounce | ✅ Done | 1s = ignored |
| Fire state in NVS | ✅ Done | Door stays open after power cut |
| Touch 2s hold = PANIC | ✅ Done | <1s ignored |
| LED SAFE green 2 Hz | ✅ Done | |
| LED WARNING yellow 2 Hz | ✅ Done | |
| LED DANGER red 2 Hz + buzzer/2s | ✅ Done | |
| LED FIRE red 5 Hz fast | ✅ Done | 200ms on/off |
| LED PANIC red-blue alternating 100ms | ✅ Done | |
| Teacher RFID silences buzzer | ✅ Done | Only if flame=0 |

---

## 4. How It Works

### Firmware

The state machine uses a 3-second hold timer — a new state must be continuously valid for 3 seconds before it takes effect. This prevents spurious state changes from sensor noise. Hysteresis prevents "chattering" at thresholds: leaving DANGER requires the temperature to drop to 38°C (not 40°C), and leaving WARNING requires both temperature ≤30°C AND humidity ≤80%.

The Rothfusz heat index equation is calculated on every sensor read and included in both the `[STATUS]` line and telemetry JSON. Heat trend tracks the last 10 temperature readings in a circular buffer; if the newest reading differs from the oldest by more than 0.5°C, it's classified as RISING or FALLING.

Flame detection uses a 2-second continuous timer — a momentary trigger (lighter waved past, not sustained) is ignored. When sustained for 2 seconds, FIRE state is set, the door opens, and `fire=true` is written to NVS. On next boot, if NVS reads `fire=true`, the door opens immediately (before WiFi connects) — matching the question paper's requirement that fire state survives a power cut.

The touch pad uses ESP32-S3's capacitive touch hardware. Holds under 1 second are silently ignored. A 2-second hold triggers PANIC and posts an alert to the cloud.

---

## 5. Demo Steps Performed

1. Warm air (hair dryer) at 32°C → waited 3s → WARNING state (yellow LED)
2. More heat at 40°C → waited 3s → DANGER (red LED + buzzer every 2s)
3. Cooled down → temperature dropped to 38°C → state changed to WARNING (hysteresis)
4. Temperature dropped to 30°C, humidity ≤80% → state changed to SAFE (green)
5. Flame sensor: waved lighter 1s → nothing happened
6. Flame sensor: held lighter 2s continuous → FIRE: red fast + door opened
7. Powered off device during FIRE → powered on → door opened immediately (NVS!)
8. Flame removed → Teacher RFID tap → buzzer silenced
9. Touch pad: tap <1s → nothing
10. Touch pad: hold 2s → PANIC state (red-blue alternating)
11. Showed `[STATUS]` line with real `HI=` and `trend=` values

---

## 6. Evidence

| Item | File Name |
|------|-----------|
| Video | `T07_L03_video.mp4` |
| Writeup PDF | `T07_L03_writeup.pdf` |
| Git Tag | `level-03` |
| Commit | [paste commit hash here] |

---

## 7. Any Deviations from Spec

- None

---

## 8. Lessons Learned

[Fill after competition]
