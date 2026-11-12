#pragma once
#include <Arduino.h>

// ── Shared state (read by any module) ─────────────────────
struct AppState {
  // Config from SmartProv / cloud
  String cloudUrl;
  String roomId;
  String classStart;
  float  tempMax      = 35.0f;
  float  humMax       = 80.0f;
  bool   buzzerOn     = true;
  bool   doorLocked   = true;

  // Live sensor readings
  float  temp         = 0.0f;
  float  hum          = 0.0f;
  bool   flame        = false;
  int    rssi         = 0;

  // Face recognition result (written by Core 1, read by Core 0)
  volatile bool  faceDetected   = false;
  volatile float faceConfidence = 0.0f;
  char           faceLabel[32]  = "";

  // Runtime flags
  bool   wifiConnected   = false;
  bool   offlineMode     = false;
};

extern AppState state;
extern SemaphoreHandle_t faceSemaphore;

