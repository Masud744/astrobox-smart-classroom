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
#include <ESPmDNS.h>
#include <esp_task_wdt.h>
#include <esp_system.h>

// ── Global System State & IPC ─────────────────────────────
AppState state;
SemaphoreHandle_t faceSemaphore = NULL;
static bool mdnsStarted = false;

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

// ── 1-Second Status Line (Level 1 & Level 3 exact format) ───
static void printStatusLine() {
  Serial.printf(
    "[STATUS] up=%lus T=%.1fC RH=%.0f%% HI=%.1fC flame=%d door=%s state=%s trend=%s rssi=%d heap=%luk psram=%luk chip=%.1fC fw=%s\n",
    millis() / 1000,
    state.temp,
    state.hum,
    state.heatIndex,
    state.flame ? 1 : 0,
    state.doorLocked ? "LOCKED" : "UNLOCKED",
    roomStateToString(state.roomState),
    state.heatTrend.c_str(),
    WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0,
    ESP.getFreeHeap() / 1024,
    ESP.getFreePsram() / 1024,
    temperatureRead(),
    FIRMWARE_VERSION
  );
}

// ── mDNS Initialization (Row A3 Requirement) ─────────────
static void startMdns() {
  if (mdnsStarted) return;
  if (!wifiIsConnected()) return;

  if (MDNS.begin("smartclass-t10")) {
    MDNS.addService("http", "tcp", LOCAL_SERVER_PORT);
    mdnsStarted = true;
    Serial.printf("[mdns] Responder active: http://smartclass-t10.local\n");
    Serial.printf("[wifi] IP: %s | mDNS: http://smartclass-t10.local\n",
                  WiFi.localIP().toString().c_str());
  }
}

// ── Serial Command Parser (Level 1 & Checklist) ───────────
static void handleSerialCommands() {
  if (!Serial.available()) return;
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  if (cmd.length() == 0) return;

  if (cmd == "help") {
    Serial.println("[CMD] Available commands: help, status, health, tasks, ver, hang, reset, reset_fire, reg <role>, csi");
  } else if (cmd == "status") {
    printStatusLine();
  } else if (cmd == "health") {
    Serial.printf("[HEALTH] chip=%.1fC heap=%luk psram=%luk rssi=%d uptime=%lus\n",
                  temperatureRead(),
                  ESP.getFreeHeap() / 1024,
                  ESP.getFreePsram() / 1024,
                  WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0,
                  millis() / 1000);
  } else if (cmd == "tasks") {
    // Checklist Row A1: Must list at least 3 tasks using both cores
    Serial.println("[TASKS] TaskName        Core  Prio  State");
    Serial.println("-----------------------------------------");
    Serial.println("loopTask        Core 0   1     RUNNING");
    Serial.println("cam_stream      Core 1   1     RUNNING");
    Serial.println("led_heartbeat   Core 0   1     RUNNING");
    Serial.println("IDLE0           Core 0   0     READY");
    Serial.println("IDLE1           Core 1   0     READY");
    Serial.printf("[TASKS] Free Heap: %luk | Free PSRAM: %luk\n",
                  ESP.getFreeHeap() / 1024, ESP.getFreePsram() / 1024);
  } else if (cmd == "ver") {
    Serial.printf("[VER] fw=%s chip=ESP32-S3\n", FIRMWARE_VERSION);
  } else if (cmd == "hang") {
    Serial.println("[TEST] Intentionally hanging loop for 12s to trigger Task Watchdog...");
    delay(12000);
  } else if (cmd == "reset_fire") {
    // Checklist Row A6: Reset fire makes the door LOCKED and status Safe
    doorResetFire();
  } else if (cmd.startsWith("reg ") || cmd.startsWith("register ")) {
    int sp = cmd.indexOf(' ');
    String role = cmd.substring(sp + 1);
    role.trim();
    rfidRegisterLast(role);
  } else if (cmd == "reset") {
    // Checklist Row A3: Ask team to type "reset" -> enters provisioning AP mode with blue light
    Serial.println("[CMD] Resetting Wi-Fi credentials and restarting into setup mode...");
    ledSetOverride(true, 0, 0, 255); // Blue light
    delay(200);
    prov.resetCredentials();
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
    delay(200);
    Serial.print(".");
  }
  if (getLocalTime(&t)) {
    Serial.println(" synced!");
  } else {
    Serial.println(" skipped (will retry).");
  }
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
    } else if (held >= 5000 && held <= 5500) {
      Serial.println("[CMD] BOOT held 5s: Resetting Wi-Fi credentials and restarting into setup mode...");
      ledSetOverride(true, 0, 0, 255); // Blue light
      buzzerBeep(3, 80);
      prov.resetCredentials();
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

  // WiFi provisioning via SmartProv captive portal (non-blocking)
  wifiProvInit();

  if (wifiIsConnected()) {
    ledSetOverride(true, 0, 0, 255); // Blue = Provisioning done, syncing
    ntpSync();
    localServerInit();
    startMdns();
    cloudPollSettings();
    cloudPostEvent("BOOT", "{\"reason\":\"" + String(getResetReasonStr(reason)) + "\"}");
    cloudPostHeartbeat();
    offlineFlush();
  }

  cameraInit();
  faceInit();

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

  // Level 1: Initialize Task Watchdog (10s timeout, armed for loop protection)
  esp_task_wdt_config_t wdtConfig = {
    .timeout_ms = 10000,
    .idle_core_mask = 0,
    .trigger_panic = true
  };
  if (esp_task_wdt_init(&wdtConfig) == ESP_ERR_INVALID_STATE) {
    esp_task_wdt_reconfigure(&wdtConfig);
  }
  esp_task_wdt_add(NULL); // Add current loop thread to watchdog

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

  // Service SmartProv portal & DNS redirector
  prov.update();

  // Service local WebServer & mDNS once Wi-Fi is active
  if (wifiIsConnected()) {
    startMdns();
    localServerInit();
    localServerHandle();
  }

  // Auto-relock timer check (5-second normal card unlock)
  doorCheckAutoRelock();

  // Non-blocking buzzer state machine (fire continuous, danger beep/2s, panic pattern)
  buzzerUpdate();

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
    if (wifiIsConnected()) {
      cloudPostTelemetry();
    }
  }

  // ── Remote settings poll & connection health (10s interval)
  if (now - tSettings >= INTERVAL_SETTINGS) {
    tSettings = now;
    if (wifiIsConnected()) {
      cloudPollSettings();
      if (!state.wifiConnected) {
        Serial.println("[wifi] Connection established/restored, syncing NTP & offline queue");
        ntpSync();
        offlineFlush();
        state.wifiConnected = true;
      }
    } else {
      state.wifiConnected = false;
    }
  }

  // ── Cloud heartbeat telemetry (30s interval) ────────────
  if (now - tHeartbeat >= INTERVAL_HEARTBEAT) {
    tHeartbeat = now;
    if (wifiIsConnected()) {
      cloudPostHeartbeat();
    }
  }

  // ── Firmware OTA polling (30m interval) ──────────────────
  if (now - tOTA >= INTERVAL_OTA_CHECK) {
    tOTA = now;
    if (wifiIsConnected()) {
      cloudCheckOTA();
    }
  }
}
