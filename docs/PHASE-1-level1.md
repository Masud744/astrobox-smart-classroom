# Phase 1 — Level 1: Dual-Core, Health, Watchdog (35 marks)
**Call 1-এ judge করাবো | Hardware পেলে শুরু করো**

> [!IMPORTANT]
> এই level-এর সব feature পরের সব level-এও কাজ করতে হবে। একবার implement করলে ভাঙলে চলবে না।

---

## 📋 Task List

### Firmware — `firmware/main/main.ino`

- [ ] **[EVENT] BOOT reason=** print করো startup-এ
  ```cpp
  #include <esp_reset_reason.h>
  // setup()-এর শুরুতে:
  const char* r;
  switch(esp_reset_reason()) {
    case ESP_RST_POWERON:  r = "POWER_ON";  break;
    case ESP_RST_SW:       r = "SOFTWARE";  break;
    case ESP_RST_TASK_WDT: r = "WATCHDOG";  break;
    case ESP_RST_PANIC:    r = "CRASH";     break;
    case ESP_RST_BROWNOUT: r = "BROWNOUT";  break;
    default:               r = "UNKNOWN";   break;
  }
  Serial.printf("[EVENT] BOOT reason=%s\n", r);
  ```

- [ ] **Task Watchdog** যোগ করো (10s timeout → restart)
  ```cpp
  #include <esp_task_wdt.h>
  // setup()-এ:
  esp_task_wdt_config_t wdt = {.timeout_ms=10000, .idle_core_mask=0, .trigger_panic=true};
  esp_task_wdt_init(&wdt);
  esp_task_wdt_add(NULL);
  // loop()-এ প্রতি iteration:
  esp_task_wdt_reset();
  ```

- [ ] **[STATUS] line প্রতি 1 সেকেন্ড** (exact format!)
  ```
  [STATUS] up=742s T=29.0C RH=64% HI=31.2C flame=0 door=LOCKED state=SAFE trend=STABLE rssi=-61 heap=182k psram=7104k chip=41.5C fw=1.0.0
  ```
  - Level 1-এ: `HI=-` এবং `trend=-` (Level 3-এ fill হবে)

- [ ] **Serial command parser** যোগ করো
  ```
  help    → সব command list
  status  → current [STATUS] line
  health  → chip temp, heap, PSRAM, uptime
  tasks   → vTaskList() output
  ver     → fw=1.0.0 chip=ESP32-S3
  hang    → 12s delay (watchdog test করতে)
  reset   → ESP.restart()
  csi     → WiFi CSI toggle (later)
  ```

### Firmware — `firmware/main/led.cpp` + `led.h`

- [ ] **2 Hz LED heartbeat** implement করো (250ms on / 250ms off)
  - FreeRTOS task-এ চালাও (blocking না করে)
  - `ledSetColor(r, g, b)` → heartbeat task এই color-এ blink করবে
  - State colors: SAFE=Green, WARNING=Yellow, DANGER=Red, ERROR=Magenta

### Firmware — `firmware/main/sensors.cpp`

- [ ] **DHT11 timeout detection**: পরপর 3টা NaN (প্রতিটা 4s gap = 12s)
  ```cpp
  static int dhtFail = 0;
  if (isnan(t) || isnan(h)) {
    if (++dhtFail >= 3) { Serial.println("[ERROR] SENSOR ERROR: DHT11"); ledSetError(); }
  } else { dhtFail = 0; }
  ```

- [ ] **RFID timeout detection**: 3s কোনো response নেই → magenta + `[ERROR] SENSOR ERROR: RFID`

### Firmware — `firmware/main/rfid.cpp`

- [ ] **[CARD] log** প্রতি tap-এ: `[CARD] uid=A3:4F:2B:11`
- [ ] **Debounce**: same card 2s-এর মধ্যে দুবার read হলে ignore

---

## 🧪 Verification Test (hardware-এ করো)

```
[ ] Serial Monitor 115200 → [EVENT] BOOT reason=POWER_ON দেখাচ্ছে
[ ] প্রতি 1s → [STATUS] line print হচ্ছে (exact format)
[ ] LED 2 Hz green heartbeat চোখে দেখা যাচ্ছে
[ ] "health" command → chip temp, heap, PSRAM দেখাচ্ছে
[ ] "tasks" command → running tasks list দেখাচ্ছে
[ ] "ver" command → fw=1.0.0 chip=ESP32-S3 দেখাচ্ছে
[ ] "hang" → 10s পরে restart → [EVENT] BOOT reason=WATCHDOG
[ ] DHT11 wire unplug → 12s পরে [ERROR] SENSOR ERROR: DHT11, LED magenta
[ ] DHT11 wire reconnect → error clear হয়, LED green ফেরে
[ ] RFID tap → [CARD] uid=... দেখাচ্ছে
[ ] Same card 2s-এর মধ্যে দুবার → শুধু একবার log
```

## ✅ Done হলে

```bash
git add -A
git commit -m "level-01: boot event, watchdog, STATUS line, serial commands, LED heartbeat, sensor timeout"
git tag level-01
git push origin main --tags
```

তারপর call দেওয়ার আগে `docs/VERIFICATION-CHECKLIST.md`-এ Level 1 section-এর সব tick দাও।
