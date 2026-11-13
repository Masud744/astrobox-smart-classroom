#include "cloud.h"
#include "app_state.h"
#include "door.h"
#include "offline.h"
#include "config.h"
#include "ota.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <Preferences.h>
#include <time.h>

static Preferences cloudPrefs;

static uint32_t getNextSeq() {
  cloudPrefs.begin("cloud", false);
  uint32_t s = cloudPrefs.getUInt("seq", 0) + 1;
  cloudPrefs.putUInt("seq", s);
  cloudPrefs.end();
  state.seq = s;
  return s;
}

// ── Low-level HTTP Helpers ────────────────────────────────

static bool httpPost(const String& path, const String& body) {
  if (WiFi.status() != WL_CONNECTED || state.cloudUrl.isEmpty()) {
    offlineQueue(path, body);
    return false;
  }

  HTTPClient http;
  http.begin(state.cloudUrl + path);
  http.addHeader("Content-Type", "application/json");
  http.setTimeout(5000);
  int code = http.POST(body);
  http.end();

  bool ok = (code == 200 || code == 201);
  if (!ok) {
    offlineQueue(path, body);
  }
  return ok;
}

static String httpGet(const String& path) {
  if (WiFi.status() != WL_CONNECTED || state.cloudUrl.isEmpty()) return "";

  HTTPClient http;
  http.begin(state.cloudUrl + path);
  http.setTimeout(5000);
  int code = http.GET();
  if (code != 200) {
    http.end();
    return "";
  }
  String res = http.getString();
  http.end();
  return res;
}

// ── Public API ────────────────────────────────────────────

bool cloudPostTelemetry() {
  uint32_t seq = getNextSeq();
  time_t nowTs = 0;
  time(&nowTs);

  JsonDocument doc;
  doc["device_id"]    = state.deviceId.isEmpty() ? "T07-D1" : state.deviceId;
  doc["room_id"]      = state.roomId.isEmpty() ? "ROOM-01" : state.roomId;
  doc["seq"]          = seq;
  doc["ts"]           = (long)nowTs;
  doc["uptime_s"]     = (long)(millis() / 1000);
  doc["temp_c"]       = serialized(String(state.temp, 1));
  doc["rh"]           = serialized(String(state.hum, 1));
  doc["heat_index_c"] = serialized(String(state.heatIndex, 1));
  doc["flame"]        = state.flame;
  doc["door"]         = state.doorLocked ? "LOCKED" : "UNLOCKED";
  doc["status"]       = roomStateToString(state.roomState);
  doc["heat_trend"]   = state.heatTrend;
  doc["rssi"]         = state.rssi;
  doc["heap_kb"]      = (int)(ESP.getFreeHeap() / 1024);
  doc["chip_temp_c"]  = serialized(String(temperatureRead(), 1));
  doc["fw"]           = FIRMWARE_VERSION;
  doc["quality"]      = state.quality;

  String body;
  serializeJson(doc, body);

  bool ok = httpPost("/api/telemetry", body);
  cloudPostSensor(); // for backward compatibility with sensor endpoints
  return ok;
}

bool cloudPostEvent(const String& type, const String& detailJson) {
  uint32_t seq = getNextSeq();
  time_t nowTs = 0;
  time(&nowTs);

  String body = "{\"device_id\":\"" + (state.deviceId.isEmpty() ? "T07-D1" : state.deviceId) + "\","
                "\"room_id\":\"" + (state.roomId.isEmpty() ? "ROOM-01" : state.roomId) + "\","
                "\"seq\":" + String(seq) + ","
                "\"ts\":" + String((long)nowTs) + ","
                "\"type\":\"" + type + "\","
                "\"detail\":" + (detailJson.isEmpty() ? "{}" : detailJson) + "}";

  return httpPost("/api/event", body);
}

bool cloudPostSensor() {
  String body = "{\"room_id\":\"" + state.roomId + "\","
                "\"temperature\":" + String(state.temp, 1) + ","
                "\"humidity\":"    + String(state.hum,  1) + ","
                "\"flame\":"       + (state.flame ? "true" : "false") + ","
                "\"wifi_rssi\":"   + String(state.rssi) + "}";
  return httpPost("/api/sensor", body);
}

bool cloudPostAttendance(const String& uid, const String& role,
                         const String& action, const String& method,
                         const String& faceLabel) {
  JsonDocument doc;
  doc["room_id"]     = state.roomId;
  doc["card_uid"]    = uid;
  doc["person_type"] = role;
  doc["action"]      = action;
  doc["method"]      = method;
  if (faceLabel.length() > 0) {
    doc["face_label"] = faceLabel;
  }
  String body;
  serializeJson(doc, body);
  return httpPost("/api/attendance", body);
}

bool cloudPostDoor(const String& doorState, const String& trigger) {
  String body = "{\"room_id\":\"" + state.roomId + "\","
                "\"state\":\""   + doorState + "\","
                "\"trigger\":\"" + trigger   + "\"}";
  return httpPost("/api/door", body);
}

bool cloudPostAlert(const String& type) {
  String body = "{\"room_id\":\"" + state.roomId + "\","
                "\"alert_type\":\"" + type + "\"}";
  return httpPost("/api/alert", body);
}

bool cloudPostHeartbeat() {
  String body = "{\"room_id\":\"" + state.roomId + "\","
                "\"firmware_version\":\"" FIRMWARE_VERSION "\","
                "\"ip_address\":\"" + WiFi.localIP().toString() + "\","
                "\"uptime_sec\":" + String(millis() / 1000) + "}";
  return httpPost("/api/heartbeat", body);
}

bool cloudPollSettings() {
  String res = httpGet("/api/settings");
  if (res.isEmpty()) return false;

  JsonDocument doc;
  if (deserializeJson(doc, res)) return false;

  bool changed = false;

  float newTempMax = doc["temp_max"] | state.tempMax;
  if (newTempMax != state.tempMax) {
    state.tempMax = newTempMax;
    changed = true;
  }

  bool resetFire = doc["reset_fire"] | false;
  if (resetFire) {
    doorResetFire();
    changed = true;
  }

  bool newDoor = doc["door_locked"] | state.doorLocked;
  if (newDoor != state.doorLocked) {
    // Door toggled remotely from web dashboard
    doorSetLocked(newDoor, "dashboard");
    changed = true;
  }

  state.humMax     = doc["hum_max"]        | state.humMax;
  state.buzzerOn   = doc["buzzer_enabled"] | state.buzzerOn;
  state.roomId     = doc["room_id"]        | state.roomId;
  state.classStart = doc["class_start"]    | state.classStart;

  return changed;
}

bool cloudCheckOTA() {
  String res = httpGet("/api/ota/latest");
  if (res.isEmpty()) return false;

  JsonDocument doc;
  if (deserializeJson(doc, res)) return false;
  if (!doc["update"].as<bool>()) return false;

  String url     = doc["url"]     | "";
  String version = doc["version"] | "";
  if (url.isEmpty()) return false;

  Serial.printf("[ota] New firmware v%s available, initiating update\n", version.c_str());
  otaFlash(url);
  return true;
}
