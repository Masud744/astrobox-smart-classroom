# Phase 3 — Level 3: Heat, Fire, Panic (50 marks)
**Call 1-এ judge করাবো — সবচেয়ে বেশি marks এখানে!**

---

## 📋 Task List

### Firmware — `firmware/main/sensors.cpp`

- [ ] **সঠিক thresholds** (আগেরটা ভুল ছিল!):
  ```cpp
  #define TEMP_WARNING       32.0f   // >= 32°C OR humidity >= 85%
  #define TEMP_DANGER        40.0f   // >= 40°C
  #define TEMP_LEAVE_DANGER  38.0f   // Hysteresis: Danger→Warning at 38°C
  #define TEMP_LEAVE_WARNING 30.0f   // Hysteresis: Warning→Safe at 30°C
  #define HUM_WARNING        85.0f
  #define HUM_LEAVE_WARNING  80.0f
  ```

- [ ] **3-second hold timer** (state change শুধু 3s ধরে থাকলে)
  ```cpp
  static StateType pending = STATE_SAFE;
  static unsigned long pendingStart = 0;

  StateType desired = calcState(temp, hum);
  if (desired != currentState) {
    if (desired == pending) {
      if (millis() - pendingStart >= 3000) currentState = desired;
    } else {
      pending = desired; pendingStart = millis();
    }
  }
  ```

- [ ] **Hysteresis** (leave state rules — Danger থেকে সরাসরি Safe-এ যাবে না)
  - Danger → Warning: temp drops to **38°C**
  - Warning → Safe: temp drops to **30°C AND** humidity ≤ 80%

- [ ] **Heat Index formula** (Rothfusz equation)
  ```cpp
  float calcHeatIndex(float T, float RH) {
    return -8.78f + 1.611f*T + 2.339f*RH - 0.1461f*T*RH
           - 0.01231f*T*T - 0.01642f*RH*RH + 0.002212f*T*T*RH
           + 0.000723f*T*RH*RH - 0.000003582f*T*T*RH*RH;
  }
  ```

- [ ] **Heat trend** (last 10 readings — circular buffer)
  ```cpp
  float hist[10]; int idx=0; bool full=false;
  // প্রতি reading-এ: hist[idx]=temp; idx=(idx+1)%10; if(!idx)full=true;
  // RISING: newest > oldest + 0.5°C
  // FALLING: newest < oldest - 0.5°C | else: STABLE
  ```

- [ ] **quality field**:
  - `"bad"` if: temp < 0 or > 50 or humidity < 20 or > 90
  - `"good"` otherwise

- [ ] **Flame 2-second continuous debounce**
  ```cpp
  static unsigned long flameStart = 0;
  bool raw = (digitalRead(PIN_FLAME) == LOW);  // Active LOW
  if (raw) {
    if (!flameStart) flameStart = millis();
    if (!fireState && millis()-flameStart >= 2000) {
      fireState = true;
      prefs.putBool("fire", true);  // NVS
      Serial.println("[EVENT] FIRE");
      doorOpen();
    }
  } else { flameStart = 0; }
  ```

- [ ] **Fire state in NVS** (power cut survive করবে)
  ```cpp
  // setup()-এ boot-এ restore:
  if (prefs.getBool("fire", false)) { fireState=true; doorOpen(); }
  ```

- [ ] **Touch: 2s hold = PANIC, <1s ignored**
  ```cpp
  static unsigned long tStart=0; static bool fired=false;
  bool on = (touchRead(PIN_TOUCH) > baseline+TOUCH_DELTA_TRIGGER);
  if (on) {
    if (!tStart) tStart=millis();
    if (!fired && millis()-tStart >= 2000) { fired=true; triggerPanic(); }
  } else {
    tStart=0; fired=false;
  }
  // <1s release → ignored automatically (no action taken)
  ```

### Firmware — `firmware/main/led.cpp`

- [ ] **State LED colors** (heartbeat task-এ):
  - `SAFE`    → Green 2 Hz
  - `WARNING` → Yellow 2 Hz
  - `DANGER`  → Red 2 Hz + buzzer beep every 2s
  - `FIRE`    → Red 5 Hz fast (200ms on/off)
  - `PANIC`   → Red-Blue alternating 100ms
  - `ERROR`   → Magenta solid

### Firmware — `firmware/main/rfid.cpp`

- [ ] Teacher card tap → **silences buzzer** (only if `flame == false`)
  - Flame still on → `Serial.println("[RFID] CANNOT SILENCE – FLAME PRESENT")`

### Firmware — `firmware/main/main.ino`

- [ ] `[STATUS]` line-এ `HI=` এবং `trend=` এখন real values দিয়ে fill করো

---

## 🧪 Verification Test

```
[ ] Hair dryer → Warning (yellow LED) exactly after 3s hold at 32°C
[ ] More heat → Danger (red + buzzer/2s) at 40°C
[ ] Cool down → Danger→Warning at 38°C (hysteresis!)
[ ] Cool more → Warning→Safe at 30°C AND humidity ≤80%
[ ] Lighter 1s → nothing happens
[ ] Lighter 2s continuous → FIRE: red fast + door opens
[ ] Power cut DURING fire → restart → door still open (NVS!)
[ ] Flame off → Teacher RFID tap → buzzer silences
[ ] Touch 1s → nothing
[ ] Touch 2s → PANIC: red-blue alternating
[ ] [STATUS] line HI= এবং trend= এখন real values দেখাচ্ছে
```

## ✅ Done হলে

```bash
git add -A
git commit -m "level-03: heat thresholds, 3s hold, hysteresis, heat index, trend, 2s flame debounce, touch 2s panic, NVS fire persist"
git tag level-03
git push origin main --tags
```

