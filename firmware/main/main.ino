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
#include <esp_task_wdt.h>
#include <esp_reset_reason.h>

// ── Global System State & IPC ─────────────────────────────
AppState state;
SemaphoreHandle_t faceSemaphore = NULL;

// ── Non-blocking Interval Timers ──────────────────────────
static unsigned long tStatus    = 0;
static unsigned long tSensor    = 0;
static unsigned long tSettings  = 0;
static unsigned long tHeartbeat = 0;
static unsigned long tOTA       = 0;

// ── Reset Reason Helper (Level 1) ─────────────────────────
static const char* getResetReasonStr(esp_reset_reason_t reason) {
  switch (reason) {
    case ESP_RST_POWERON:   return "POWER_ON";
    case ESP_RST_SW:        return "SOFTWARE";
    case ESP_RST_PANIC:     return "CRASH";
    case ESP_RST_INT_WDT:
    case ESP_RST_TASK_WDT:
    case ESP_RST_WDT:       return "WATCHDOG";
    case ESP_RST_DEEPSLEEP: return "DEEP_SLEEP";
    case ESP_RST_BROWNOUT:  return "BROWNOUT";
    case ESP_RST_SDIO:      return "SDIO";
    default:                return "UNKNOWN";
  }
}

// ── 1-Second Status Line (Level 1 exact format) ───────────
static void printStatusLine() {
  // Exact question paper format:
  // [STATUS] up=742s T=29.0C RH=64% HI=- flame=0 door=LOCKED state=SAFE trend=- rssi=-61 heap=182k psram=7104k chip=41.5C fw=1.0.0
  Serial.printf(
    "[STATUS] up=%lus T=%.1fC RH=%.0f%% HI=- flame=%d door=%s state=%s trend=- rssi=%d heap=%luk psram=%luk chip=%.1fC fw=%s\n",
    millis() / 1000,
    state.temp,
    state.hum,
    state.flame ? 1 : 0,
    state.doorLocked ? "LOCKED" : "OPEN",
    roomStateToString(state.roomState),
    WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0,
    ESP.getFreeHeap() / 1024,
    ESP.getFreePsram() / 1024,
    temperatureRead(),
    FIRMWARE_VERSION
  );
}

// ── Serial Command Parser (Level 1) ───────────────────────
static void handleSerialCommands() {
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  if (cmd.length() == 0) return;

  if (cmd == "help") {
    Serial.println("[CMD] Available commands: help, status, health, tasks, ver, hang, reset, csi");
  } else if (cmd == "status") {
    printStatusLine();
  } else if (cmd == "health") {
    Serial.printf("[HEALTH] chip=%.1fC heap=%luk psram=%luk uptime=%lus\n",
                  temperatureRead(),
                  ESP.getFreeHeap() / 1024,
                  ESP.getFreePsram() / 1024,
                  millis() / 1000);
  } else if (cmd == "tasks") {
    #if (configUSE_TRACE_FACILITY == 1)
    char taskListBuf[1024];
    vTaskList(taskListBuf);
    Serial.println("[TASKS] Name          State  Prio   Stack   Num");
    Serial.println("----------------------------------------------");
    Serial.print(taskListBuf);
    #else
    Serial.printf("[TASKS] Core0 loop active, Cam Core1 active. Free Heap: %u B, PSRAM: %u B\n",
                  ESP.getFreeHeap(), ESP.getFreePsram());
    #endif
  } else if (cmd == "ver") {
    Serial.printf("[VER] fw=%s chip=ESP32-S3\n", FIRMWARE_VERSION);
  } else if (cmd == "hang") {
    Serial.println("[TEST] Intentionally hanging loop for 12s to trigger Task Watchdog...");
    delay(12000);
  } else if (cmd == "reset") {
    Serial.println("[CMD] Resetting ESP32-S3...");
    delay(100);
    ESP.restart();
  } else if (cmd == "csi") {
    Serial.println("[CSI] Wi-Fi CSI toggle (Level 11 feature)");
  } else {
    Serial.printf("[CMD] Unknown command: '%s'. Type 'help' for command list.\n", cmd.c_str());
  }
}

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
      faceClearAll();
      buzzerBeep(5, 80);
      pressed = false;
    } else if (held > 3000 && held <= 3200) {
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

  // Level 1: Boot event with exact reset reason
  esp_reset_reason_t reason = esp_reset_reason();
  Serial.printf("[EVENT] BOOT reason=%s\n", getResetReasonStr(reason));

  Serial.println("\n==============================================");
  Serial.println("  FastRobox AI-Powered Smart Classroom v" FIRMWARE_VERSION);
  Serial.println("==============================================");

  // Level 1: Initialize Task Watchdog (10s timeout, panic on trigger)
  esp_task_wdt_config_t wdtConfig = {
    .timeout_ms = 10000,
    .idle_core_mask = 0,
    .trigger_panic = true
  };
  esp_task_wdt_init(&wdtConfig);
  esp_task_wdt_add(NULL); // Add current Core 0 thread to watchdog

  // Initialize status LED and start background 2 Hz FreeRTOS heartbeat task
  ledInit();
  ledSetOverride(true, 255, 165, 0); // Orange = Booting
  ledStartHeartbeat();

  buzzerInit();
  pinMode(PIN_BOOT, INPUT_PULLUP);

  offlineInit();
  sensorsInit();
  rfidInit();
  doorInit();

  // WiFi provisioning via SmartProv captive portal
  wifiProvInit();
  ledSetOverride(true, 0, 0, 255); // Blue = Provisioning done, syncing

  ntpSync();
  cameraInit();
  faceInit();
  localServerInit();

  // Core 1 dedicated camera stream & face recognition task
  faceSemaphore = xSemaphoreCreateMutex();
  xTaskCreatePinnedToCore(
    cameraStreamTask,
    "cam_stream",
    16384,
    NULL,
    1,
    NULL,
    1
  );

  // Initial cloud polling and heartbeat
  cloudPollSettings();
  cloudPostHeartbeat();
  offlineFlush();

  // Ready: Release LED override to start 2 Hz heartbeat reflecting system state
  ledSetOverride(false);
  buzzerBeep(1, 100);
  Serial.println("[ready] All systems operational and ready");
}

// ══════════════════════════════════════════════════════════
// LOOP (Runs on Core 0 / Arduino Task)
// ══════════════════════════════════════════════════════════
void loop() {
  // Feed the Task Watchdog on every iteration
  esp_task_wdt_reset();

  // Service Serial Command Parser (Level 1)
  handleSerialCommands();

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

  // ── Level 1: 1-Second STATUS line ─────────────────────────
  if (now - tStatus >= 1000) {
    tStatus = now;
    printStatusLine();
  }

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

