#include "cloud.h"
#include "app_state.h"
#include "door.h"
#include "offline.h"
#include "config.h"
#include "ota.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>

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
