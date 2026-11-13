#include "led.h"
#include "app_state.h"
#include "config.h"
#include "wifi_prov.h"

static bool overrideActive = false;
static uint8_t ovR = 0, ovG = 0, ovB = 0;
static TaskHandle_t heartbeatTaskHandle = NULL;

void ledInit() {
#ifdef RGB_PWR
  pinMode(RGB_PWR, OUTPUT);
  digitalWrite(RGB_PWR, HIGH);
#endif
  ledSet(0, 0, 0);
}

void ledSet(uint8_t r, uint8_t g, uint8_t b) {
  rgbLedWrite(PIN_RGB_LED, r, g, b);
}

void ledSetOverride(bool active, uint8_t r, uint8_t g, uint8_t b) {
  overrideActive = active;
  ovR = r;
  ovG = g;
  ovB = b;
  if (active) {
    ledSet(r, g, b);
  }
}

// ── FreeRTOS 2 Hz Heartbeat Task (250ms ON / 250ms OFF) ───
// Runs independently on Core 0 to provide non-blocking visual telemetry
static void ledHeartbeatTask(void* param) {
  bool altState = false;

  while (true) {
    if (overrideActive) {
      ledSet(ovR, ovG, ovB);
      vTaskDelay(pdMS_TO_TICKS(100));
      continue;
    }

    // Determine active color based on diagnostic errors or room state
    uint8_t r = 0, g = 0, b = 0;
    int onMs = 250;
    int offMs = 250;

    if (state.dhtError || state.rfidError) {
      // Magenta alert for hardware sensor fault (Level 1 spec)
      r = 255; g = 0; b = 255;
    } else if (prov.isSetupMode() || (!state.wifiConnected && !wifiIsConnected())) {
      // Blue 2 Hz heartbeat in setup mode (Level 2 Row A3 & Test 3)
      r = 0; g = 0; b = 255;
    } else {
      switch (state.roomState) {
        case STATE_SAFE:
          r = 0; g = 255; b = 0;    // Green 2 Hz
          break;
        case STATE_WARNING:
          r = 255; g = 180; b = 0;  // Yellow 2 Hz
          break;
        case STATE_DANGER:
          r = 255; g = 0; b = 0;    // Red 2 Hz
          break;
        case STATE_FIRE:
          r = 255; g = 0; b = 0;    // Red 5 Hz fast flash
          onMs = 100;
          offMs = 100;
          break;
        case STATE_PANIC:
          // Alternating Red / Blue 100ms
          altState = !altState;
          if (altState) { r = 255; g = 0; b = 0; }
          else          { r = 0; g = 0; b = 255; }
          onMs = 100;
          offMs = 0; // immediate transition in alternating cycle
          break;
      }
    }

    // Phase 1: LED ON
    ledSet(r, g, b);
    vTaskDelay(pdMS_TO_TICKS(onMs));

    // Phase 2: LED OFF (heartbeat blink)
    if (offMs > 0) {
      ledSet(0, 0, 0);
      vTaskDelay(pdMS_TO_TICKS(offMs));
    }
  }
}

void ledStartHeartbeat() {
  if (heartbeatTaskHandle == NULL) {
    xTaskCreatePinnedToCore(
      ledHeartbeatTask,
      "led_hb",
      2048,
      NULL,
      1,
      &heartbeatTaskHandle,
      0
    );
  }
}

