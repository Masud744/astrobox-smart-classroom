#include "sensors.h"
#include "app_state.h"
#include "cloud.h"
#include "led.h"
#include "buzzer.h"
#include "config.h"
#include <DHT.h>
#include <WiFi.h>

static DHT dht(PIN_DHT, DHT11);
static bool prevFlame = false;
static bool prevTouch = false;
static uint32_t touchBaseline = 0;
static unsigned long lastValidDht = 0;

void sensorsInit() {
  pinMode(PIN_FLAME, INPUT);
  dht.begin();
  lastValidDht = millis();

  // Auto-calibrate capacitive touch baseline on ESP32-S3
  uint32_t sum = 0;
  for (int i = 0; i < 10; i++) {
    sum += touchRead(PIN_TOUCH);
    delay(20);
  }
  touchBaseline = sum / 10;
  Serial.printf("[sensors] DHT11(pin %d), Flame(pin %d), Touch baseline: %u\n",
                PIN_DHT, PIN_FLAME, touchBaseline);
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
  } else {
    if (state.dhtError) {
      state.dhtError = false;
      Serial.println("[INFO] SENSOR RECOVERED: DHT11");
    }
    lastValidDht = millis();
    state.temp = t;
    state.hum  = h;
  }

  state.rssi = WiFi.RSSI();

  Serial.printf("[sensor] Temp: %.1f°C | Hum: %.1f%% | RSSI: %d dBm\n",
                state.temp, state.hum, state.rssi);

  // Trigger threshold alerts if exceeded
  if (state.temp > state.tempMax) cloudPostAlert("temp_high");
  if (state.hum  > state.humMax)  cloudPostAlert("hum_high");
}

void flameCheck() {
  // IR flame sensor digital out: active LOW (LOW indicates flame presence)
  bool flame = (digitalRead(PIN_FLAME) == LOW);

  if (flame && !prevFlame) {
    Serial.println("[flame] ALERT! Flame detected!");
    state.roomState = STATE_FIRE;
    buzzerAlarm();
    cloudPostAlert("flame");
  } else if (!flame && prevFlame) {
    if (state.roomState == STATE_FIRE) {
      state.roomState = STATE_SAFE;
    }
  }

  state.flame = flame;
  prevFlame   = flame;
}

void touchCheck() {
  // ESP32-S3 touch hardware reading increases upon contact
  uint32_t val = touchRead(PIN_TOUCH);
  bool touched = (val > (touchBaseline + TOUCH_DELTA_TRIGGER));

  if (touched && !prevTouch) {
    Serial.printf("[panic] Touch panic pad triggered! (val: %u, base: %u)\n",
                  val, touchBaseline);
    state.roomState = STATE_PANIC;
    buzzerBeep(4, 120);
    cloudPostAlert("panic");
  } else if (!touched && prevTouch) {
    if (state.roomState == STATE_PANIC) {
      state.roomState = STATE_SAFE;
    }
  }

  prevTouch = touched;
}
