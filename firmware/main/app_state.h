#pragma once
#include <Arduino.h>

enum RoomState {
  STATE_SAFE,
  STATE_WARNING,
  STATE_DANGER,
  STATE_FIRE,
  STATE_PANIC
};

inline const char* roomStateToString(RoomState s) {
  switch (s) {
    case STATE_SAFE:    return "SAFE";
    case STATE_WARNING: return "WARNING";
    case STATE_DANGER:  return "DANGER";
    case STATE_FIRE:    return "FIRE";
    case STATE_PANIC:   return "PANIC";
    default:            return "UNKNOWN";
  }
}

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

  // Live sensor readings & states
  float     temp         = 0.0f;
  float     hum          = 0.0f;
  bool      flame        = false;
  int       rssi         = 0;
  RoomState roomState    = STATE_SAFE;

  // Sensor diagnostic error flags (Level 1)
  bool dhtError  = false;
  bool rfidError = false;

  // Face recognition result (written by Core 1, read by Core 0)
  volatile bool  faceDetected   = false;
  volatile float faceConfidence = 0.0f;
  char           faceLabel[32]  = "";

  // Level 3 Live metrics & diagnostics
  float     heatIndex    = 0.0f;
  String    heatTrend    = "STABLE";
  String    quality      = "good";
  String    lastSilenceRefusedMsg = "";

  // Device & Telemetry identification (Level 2)
  String        deviceId       = "T10-D1";
  String        deviceToken    = "";
  uint32_t      seq            = 0;
  bool          cloudReachable = false;

  // Runtime flags
  bool wifiConnected = false;
  bool offlineMode   = false;

  // Last scanned card (Level 2 A4 requirement)
  String        lastCardUid  = "-";
  unsigned long lastCardTime = 0;
};

extern AppState state;
extern SemaphoreHandle_t faceSemaphore;

