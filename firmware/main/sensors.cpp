#include "sensors.h"
#include "app_state.h"
#include "cloud.h"
#include "door.h"
#include "led.h"
#include "buzzer.h"
#include "config.h"
#include <DHT.h>
#include <WiFi.h>

// Level 3 Thresholds
#define TEMP_WARNING       32.0f   // >= 32°C OR humidity >= 85%
#define TEMP_DANGER        40.0f   // >= 40°C
#define TEMP_LEAVE_DANGER  38.0f   // Hysteresis: Danger->Warning at 38°C
#define TEMP_LEAVE_WARNING 30.0f   // Hysteresis: Warning->Safe at 30°C
#define HUM_WARNING        85.0f   // >= 85%
#define HUM_LEAVE_WARNING  80.0f   // <= 80%

static DHT dht(PIN_DHT, DHT11);
static bool prevFlame = false;
static uint32_t touchBaseline1 = 0;
static uint32_t touchBaseline4 = 0;
static unsigned long lastValidDht = 0;
static unsigned long flameStartTime = 0;
static unsigned long touchStartTime = 0;
static unsigned long panicStartTime = 0;

static float tempHist[10];
static int tempHistCount = 0;
static int tempHistIdx = 0;

static RoomState pendingThermal = STATE_SAFE;
static unsigned long pendingThermalStart = 0;

// Rothfusz equation for Heat Index
static float calcHeatIndex(float T, float RH) {
  return -8.784695f + 1.61139411f * T + 2.338549f * RH - 0.14611605f * T * RH
         - 0.012308094f * T * T - 0.016424828f * RH * RH + 0.002211732f * T * T * RH
         + 0.00072546f * T * RH * RH - 0.000003582f * T * T * RH * RH;
}

static void recordTemp(float t) {
  tempHist[tempHistIdx] = t;
  tempHistIdx = (tempHistIdx + 1) % 10;
  if (tempHistCount < 10) tempHistCount++;
}

static String calcHeatTrend() {
  if (tempHistCount < 2) return "STABLE";
  int oldestIdx = (tempHistCount < 10) ? 0 : tempHistIdx;
  int newestIdx = (tempHistIdx - 1 + 10) % 10;
  float diff = tempHist[newestIdx] - tempHist[oldestIdx];
  if (diff > 0.5f) return "RISING";
  if (diff < -0.5f) return "FALLING";
  return "STABLE";
}

static RoomState calcThermalState(float t, float h, RoomState current) {
  if (current == STATE_DANGER) {
    if (t < TEMP_LEAVE_DANGER) return STATE_WARNING;
    return STATE_DANGER;
  } else if (current == STATE_WARNING) {
    if (t >= TEMP_DANGER) return STATE_DANGER;
    if (t <= TEMP_LEAVE_WARNING && h <= HUM_LEAVE_WARNING) return STATE_SAFE;
    return STATE_WARNING;
  } else {
    // Current is SAFE
    if (t >= TEMP_DANGER) return STATE_DANGER;
    if (t >= TEMP_WARNING || h >= HUM_WARNING) return STATE_WARNING;
    return STATE_SAFE;
  }
}

static void updateThermalState(float t, float h) {
  if (state.roomState == STATE_FIRE || state.roomState == STATE_PANIC) return;

  RoomState desired = calcThermalState(t, h, state.roomState);
  if (desired != state.roomState) {
    if (desired == pendingThermal) {
      if (millis() - pendingThermalStart >= 3000) {
        RoomState oldState = state.roomState;
        state.roomState = desired;
        Serial.printf("[EVENT] STATUS %s->%s\n", roomStateToString(oldState), roomStateToString(desired));
        cloudPostEvent("STATUS", "{\"old\":\"" + String(roomStateToString(oldState)) + "\",\"new\":\"" + String(roomStateToString(desired)) + "\"}");
        pendingThermalStart = 0;
      }
    } else {
      pendingThermal = desired;
      pendingThermalStart = millis();
    }
  } else {
    pendingThermalStart = 0;
  }
}

void sensorsInit() {
  pinMode(PIN_FLAME, INPUT);
  dht.begin();
  lastValidDht = millis();

  // Auto-calibrate capacitive touch baseline on ESP32-S3 (supports both GPIO 1 & GPIO 4)
  uint32_t sum1 = 0, sum4 = 0;
  for (int i = 0; i < 10; i++) {
    sum1 += touchRead(1);
    sum4 += touchRead(4);
    delay(20);
  }
  touchBaseline1 = sum1 / 10;
  touchBaseline4 = sum4 / 10;
  Serial.printf("[sensors] DHT11(pin %d), Flame(pin %d), Touch baseline (GPIO 1): %u, (GPIO 4): %u\n",
                PIN_DHT, PIN_FLAME, touchBaseline1, touchBaseline4);
}

void sensorsRead() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  // DHT11 12-second timeout detection (Question Paper Requirement)
  if (isnan(t) || isnan(h)) {
    if (millis() - lastValidDht >= 12000 && !state.dhtError) {
      state.dhtError = true;
      Serial.println("[ERROR] SENSOR ERROR: DHT11");
    }
    state.quality = "bad";
  } else {
    if (state.dhtError) {
      state.dhtError = false;
      Serial.println("[INFO] SENSOR RECOVERED: DHT11");
    }
    lastValidDht = millis();

    // Check valid reading ranges (0-50 C, 20-90% RH)
    bool outOfRange = (t < 0.0f || t > 50.0f || h < 20.0f || h > 90.0f);
    if (outOfRange) {
      state.quality = "bad";
    } else {
      state.quality = "good";
      state.temp = t;
      state.hum  = h;
      state.heatIndex = calcHeatIndex(t, h);
      recordTemp(t);
      state.heatTrend = calcHeatTrend();
      updateThermalState(t, h);
    }
  }

  state.rssi = WiFi.RSSI();

  Serial.printf("[sensor] Temp: %.1f°C | Hum: %.1f%% | HI: %.1f°C | Trend: %s | Q: %s\n",
                state.temp, state.hum, state.heatIndex, state.heatTrend.c_str(), state.quality.c_str());

  // Trigger threshold alerts if exceeded (NOTE: Temperature NEVER unlocks the door)
  if (state.temp > state.tempMax) cloudPostAlert("temp_high");
  if (state.hum  > state.humMax)  cloudPostAlert("hum_high");
}

void flameCheck() {
  // IR flame sensor digital out: active LOW (LOW indicates flame presence)
  bool flame = (digitalRead(PIN_FLAME) == LOW);
  unsigned long now = millis();

  if (flame) {
    if (flameStartTime == 0) flameStartTime = now;

    // Requirement: Continuous flame for 2 seconds confirms emergency
    if (now - flameStartTime >= 2000 && state.roomState != STATE_FIRE) {
      Serial.println("[EVENT] FIRE");
      state.roomState = STATE_FIRE;
      doorUnlockPermanent("flame"); // Permanently UNLOCKED until "Reset fire"
      buzzerAlarm();                 // Continuous buzzer
      cloudPostEvent("FIRE", "{\"trigger\":\"flame_2s\"}");
      cloudPostAlert("flame");
    }
  } else {
    if (prevFlame && state.roomState == STATE_FIRE) {
      Serial.println("[EVENT] FIRE_CLEARED");
      cloudPostEvent("FIRE_CLEARED", "{}");
    }
    flameStartTime = 0;
    // When flame is put out: DO NOT auto-silence the buzzer or lock door!
    // Only the Teacher RFID tag can silence the buzzer.
    // Only "Reset fire" command can re-lock the door.
  }

  state.flame = flame;
  prevFlame   = flame;
}

void touchCheck() {
  unsigned long now = millis();

  // Test 7: Red & Blue light and buzzer alternate for 10 seconds during PANIC, then restore to SAFE
  if (state.roomState == STATE_PANIC) {
    if (now - panicStartTime >= 10000) {
      state.roomState = STATE_SAFE;
      Serial.println("[panic] PANIC alert cycle finished (10s). State returned to SAFE.");
      touchStartTime = 0;
    }
    return;
  }

  // ESP32-S3 capacitive touch reading increases on finger contact
  // Supports wire connected to either GPIO 4 or GPIO 1
  uint32_t val1 = touchRead(1);
  uint32_t val4 = touchRead(4);
  bool touched = (val1 > (touchBaseline1 + TOUCH_DELTA_TRIGGER)) ||
                 (val4 > (touchBaseline4 + TOUCH_DELTA_TRIGGER));

  // Touching the pad for 2 seconds raises PANIC (<1s ignored as false alarm filter)
  if (touched) {
    if (touchStartTime == 0) touchStartTime = now;

    if (now - touchStartTime >= 2000 && state.roomState != STATE_PANIC) {
      Serial.println("[EVENT] PANIC");
      state.roomState = STATE_PANIC;
      panicStartTime = now;
      cloudPostEvent("PANIC", "{\"trigger\":\"touch_2s\"}");
      cloudPostAlert("panic");
      // Guard: Door DOES NOT unlock for panic wire touch
    }
  } else {
    touchStartTime = 0;
  }
}
