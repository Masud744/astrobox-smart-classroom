#include "config.h"
#include "app_state.h"
#include "wifi_prov.h"
#include "sensors.h"
#include "rfid.h"
#include "door.h"
#include "cloud.h"
#include "local_server.h"
#include "camera.h"
#include "face.h"
#include "ota.h"
#include "offline.h"
#include "led.h"
#include "buzzer.h"
#include <time.h>
#include <SmartProv.h>

// ── Global System State & IPC ─────────────────────────────
AppState state;
SemaphoreHandle_t faceSemaphore = NULL;

// ── Non-blocking Interval Timers ──────────────────────────
static unsigned long tSensor    = 0;
static unsigned long tSettings  = 0;
static unsigned long tHeartbeat = 0;
static unsigned long tOTA       = 0;

// ── NTP Clock Sync ────────────────────────────────────────
static void ntpSync() {
  configTime(6 * 3600, 0, "pool.ntp.org", "time.google.com");
  Serial.print("[ntp]  Syncing network time");
  struct tm t;
  for (int i = 0; i < 20 && !getLocalTime(&t); i++) {
    delay(300);
    Serial.print(".");
  }
  Serial.println(" synced!");
}

// ── BOOT Button: Long-press Actions ───────────────────────
static void checkBootButton() {
  static unsigned long pressStart = 0;
  static bool pressed = false;

  if (digitalRead(PIN_BOOT) == LOW) {
    if (!pressed) {
      pressStart = millis();
      pressed = true;
    }

    unsigned long held = millis() - pressStart;
    if (held > 10000) {
      // 10s hold: wipe all biometric face embeddings (privacy rule)
      faceClearAll();
      buzzerBeep(5, 80);
      pressed = false;
    } else if (held > 3000 && held <= 3200) {
      // 3s hold: trigger manual OTA update check
      buzzerBeep(2, 80);
      cloudCheckOTA();
    }
  } else {
    pressed = false;
  }
}

// ══════════════════════════════════════════════════════════
// SETUP (Runs on Core 1 by default on ESP32 Arduino)
// ══════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n==============================================");
  Serial.println("  FastRobox AI-Powered Smart Classroom v" FIRMWARE_VERSION);
  Serial.println("==============================================");

  // Initialize status LED (Orange = booting)
  ledInit();
  ledSet(255, 165, 0);

  buzzerInit();
  pinMode(PIN_BOOT, INPUT_PULLUP);

  offlineInit();
  sensorsInit();
  rfidInit();
  doorInit();

  // WiFi provisioning via SmartProv captive portal
  wifiProvInit();
  ledSet(0, 0, 255);   // Blue = provisioning done, connecting & syncing

  ntpSync();
  cameraInit();
  faceInit();
  localServerInit();

  // Core 1 dedicated camera stream & face recognition task
  faceSemaphore = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(
    cameraStreamTask,  // Task entry function
    "cam_stream",      // Task name
    16384,             // 16KB stack for camera + face processing
    NULL,              // Task parameter
    1,                 // Task priority
    NULL,              // Task handle
    1                  // Pinned to Core 1
  );

  // Initial cloud polling and heartbeat
  cloudPollSettings();
  cloudPostHeartbeat();
  offlineFlush();

  // Status LED reflects door state (Red = Locked, Green = Unlocked)
  ledSet(state.doorLocked ? 255 : 0, state.doorLocked ? 0 : 255, 0);
  buzzerBeep(1, 100);
  Serial.println("[ready] All systems operational and ready");
}

// ══════════════════════════════════════════════════════════
// LOOP (Runs on Core 0 / Arduino Task)
// ══════════════════════════════════════════════════════════
void loop() {
  // Service SmartProv portal & local WebServer
  prov.update();
  localServerHandle();

  // Continuous sensor & security monitoring
  flameCheck();
  touchCheck();
  rfidCheck();
  checkBootButton();

  // Check face recognition signals emitted by Core 1
  if (state.faceDetected) {
    if (xSemaphoreTake(faceSemaphore, 5) == pdTRUE) {
      Serial.printf("[face] Acknowledged face: %s (confidence: %.2f)\n",
                    state.faceLabel, state.faceConfidence);
      state.faceDetected = false;
      xSemaphoreGive(faceSemaphore);
    }
  }

  unsigned long now = millis();

  // ── Sensor reading & cloud telemetry (5s interval) ───────
  if (now - tSensor >= INTERVAL_SENSOR) {
    tSensor = now;
    sensorsRead();
    cloudPostSensor();
  }

  // ── Remote settings poll & connection health (10s interval)
  if (now - tSettings >= INTERVAL_SETTINGS) {
    tSettings = now;
    cloudPollSettings();

    bool wifiNow = wifiIsConnected();
    if (!state.wifiConnected && wifiNow) {
      // Reconnected after drop — flush offline queue
      Serial.println("[wifi] Connection restored, flushing offline queue");
      offlineFlush();
    }
    state.wifiConnected = wifiNow;
  }

  // ── Cloud heartbeat telemetry (30s interval) ────────────
  if (now - tHeartbeat >= INTERVAL_HEARTBEAT) {
    tHeartbeat = now;
    cloudPostHeartbeat();
  }

  // ── Firmware OTA polling (30m interval) ──────────────────
  if (now - tOTA >= INTERVAL_OTA_CHECK) {
    tOTA = now;
    cloudCheckOTA();
  }
}
