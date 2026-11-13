#include "local_server.h"
#include "app_state.h"
#include "door.h"
#include "rfid.h"
#include "config.h"
#include "wifi_prov.h"
#include <WebServer.h>
#include <ArduinoJson.h>
#include <WiFi.h>

static WebServer server(LOCAL_SERVER_PORT);
static bool localServerRunning = false;

// ── Route Handlers ────────────────────────────────────────

static void handleRoot() {
  String html = R"(<!DOCTYPE html><html>
<head><meta charset='UTF-8'><meta http-equiv='refresh' content='3'>
<title>FastRobox Smart Classroom</title>
<style>
  body{font-family:system-ui,-apple-system,sans-serif;background:#0d1117;color:#c9d1d9;padding:2rem;max-width:650px;margin:auto}
  h2{color:#a855f7;border-bottom:1px solid #30363d;padding-bottom:.5rem}
  .card{background:#161b22;border:1px solid #30363d;border-radius:8px;padding:1rem;margin-bottom:1rem}
  .row{display:flex;justify-content:space-between;padding:.3rem 0}
  .ok{color:#22c55e;font-weight:bold}
  .warn{color:#ef4444;font-weight:bold}
  .btn{background:#7c3aed;color:#fff;border:none;padding:.5rem 1rem;border-radius:6px;cursor:pointer;font-size:1rem;margin-top:.5rem}
</style>
</head>
<body>
<h2>FastRobox — )" + state.roomId + R"(</h2>
<div class='card'>
  <div class='row'><span>Temperature:</span><b>)" + String(state.temp, 1) + R"( °C</b></div>
  <div class='row'><span>Humidity:</span><b>)" + String(state.hum, 1) + R"( %</b></div>
  <div class='row'><span>Flame Status:</span><span class=')" + String(state.flame ? "warn" : "ok") + R"('>)" + String(state.flame ? "🔥 DETECTED" : "✅ SAFE") + R"(</span></div>
  <div class='row'><span>Door Status:</span><span class=')" + String(state.doorLocked ? "warn" : "ok") + R"('>)" + String(state.doorLocked ? "🔒 LOCKED" : "🔓 UNLOCKED") + R"(</span></div>
  <div class='row'><span>WiFi RSSI:</span><b>)" + String(state.rssi) + R"( dBm</b></div>
  <div class='row'><span>Uptime:</span><b>)" + String(millis() / 1000) + R"( s</b></div>
  <div class='row'><span>Firmware:</span><b>v)" FIRMWARE_VERSION R"(</b></div>
  <div class='row'><span>Cloud:</span><small>)" + state.cloudUrl + R"(</small></div>
</div>
<form action='/door' method='POST' style='display:inline'>
  <button class='btn' type='submit'>Toggle Door</button>
</form>
<a href='/stream' target='_blank' style='margin-left:1rem'><button class='btn' type='button' style='background:#2563eb'>Live Camera</button></a>
</body></html>)";

  server.send(200, "text/html", html);
}

static void handleStatusJson() {
  JsonDocument doc;
  doc["room_id"]     = state.roomId;
  doc["temperature"] = state.temp;
  doc["humidity"]    = state.hum;
  doc["flame"]       = state.flame;
  doc["door_locked"] = state.doorLocked;
  doc["wifi_rssi"]   = state.rssi;
  doc["uptime_sec"]  = millis() / 1000;
  doc["firmware"]    = FIRMWARE_VERSION;
  doc["wifi_status"] = state.wifiConnected;

  String out;
  serializeJson(doc, out);
  server.send(200, "application/json", out);
}

static void handleDoorToggle() {
  doorToggle("local_http");
  server.sendHeader("Location", "/");
  server.send(303);
}

static void handleRfidRegister() {
  if (!server.hasArg("plain")) {
    server.send(400, "application/json", "{\"error\":\"Missing body\"}");
    return;
  }

  JsonDocument doc;
  if (deserializeJson(doc, server.arg("plain"))) {
    server.send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
    return;
  }

  String uid  = doc["uid"]  | "";
  String role = doc["role"] | "student";

  if (uid.isEmpty()) {
    server.send(400, "application/json", "{\"error\":\"uid required\"}");
    return;
  }

  rfidRegister(uid, role);
  server.send(200, "application/json", "{\"ok\":true,\"uid\":\"" + uid + "\",\"role\":\"" + role + "\"}");
}

static void handleStream() {
  server.sendHeader("Location", "http://" + WiFi.localIP().toString() + ":" + String(CAMERA_STREAM_PORT) + "/stream");
  server.send(302);
}

// ── Public API ────────────────────────────────────────────

void localServerInit() {
  if (localServerRunning) return;
  if (!wifiIsConnected()) return; // Prevent port 80 collision with SmartProv in AP mode

  server.on("/",              HTTP_GET,  handleRoot);
  server.on("/status.json",   HTTP_GET,  handleStatusJson);
  server.on("/door",          HTTP_POST, handleDoorToggle);
  server.on("/rfid/register", HTTP_POST, handleRfidRegister);
  server.on("/stream",        HTTP_GET,  handleStream);
  server.begin();
  localServerRunning = true;
  Serial.printf("[http] Local server active at http://%s:%d\n",
                WiFi.localIP().toString().c_str(), LOCAL_SERVER_PORT);
}

void localServerHandle() {
  if (localServerRunning) {
    server.handleClient();
  }
}
