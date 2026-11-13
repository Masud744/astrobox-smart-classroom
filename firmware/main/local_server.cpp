#include "local_server.h"
#include "app_state.h"
#include "door.h"
#include "rfid.h"
#include "config.h"
#include "wifi_prov.h"
#include <WebServer.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <time.h>

static WebServer server(LOCAL_SERVER_PORT);
static bool localServerRunning = false;

// ── Helpers ───────────────────────────────────────────────

static String getDhakaTime() {
  struct tm t;
  if (!getLocalTime(&t)) {
    return "--:--:--";
  }
  char buf[32];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
  return String(buf);
}

static String getDisplayStatus() {
  if (state.roomState == STATE_FIRE) return "FIRE – EVACUATE";
  if (state.roomState == STATE_PANIC) return "PANIC";
  return String(roomStateToString(state.roomState));
}

static String getConnectionState() {
  return state.wifiConnected ? "LIVE" : "OFFLINE";
}

// ── Route Handlers ────────────────────────────────────────

static void handleRoot() {
  String html = R"(<!DOCTYPE html><html>
<head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>
<meta http-equiv='refresh' content='2'>
<title>FastRobox Smart Classroom</title>
<style>
  body{font-family:system-ui,-apple-system,sans-serif;background:#0d1117;color:#c9d1d9;padding:1.5rem;max-width:700px;margin:auto}
  h2{color:#a855f7;border-bottom:1px solid #30363d;padding-bottom:.5rem;margin-top:0}
  .card{background:#161b22;border:1px solid #30363d;border-radius:8px;padding:1rem;margin-bottom:1rem}
  .row{display:flex;justify-content:space-between;padding:.35rem 0;border-bottom:1px solid #21262d}
  .row:last-child{border-bottom:none}
  .ok{color:#22c55e;font-weight:bold}
  .warn{color:#ef4444;font-weight:bold}
  .alert-banner{background:#b91c1c;color:#fff;padding:.8rem;border-radius:6px;font-weight:bold;margin-bottom:1rem;text-align:center;font-size:1.1rem;letter-spacing:1px}
  .refuse-banner{background:#d97706;color:#fff;padding:.8rem;border-radius:6px;font-weight:bold;margin-bottom:1rem;text-align:center}
  .btn{background:#7c3aed;color:#fff;border:none;padding:.5rem 1rem;border-radius:6px;cursor:pointer;font-size:.95rem;margin-top:.5rem;text-decoration:none;display:inline-block}
  .btn-green{background:#15803d}
  .btn-blue{background:#2563eb}
  .btn-amber{background:#b45309}
</style>
</head>
<body>
<h2>FastRobox — )" + (state.roomId.isEmpty() ? "ROOM-01" : state.roomId) + R"(</h2>)";

  if (state.roomState == STATE_FIRE) {
    html += "<div class='alert-banner'>🔥 FIRE – EVACUATE</div>";
  } else if (state.roomState == STATE_PANIC) {
    html += "<div class='alert-banner'>⚠️ PANIC ALERT ACTIVE</div>";
  }

  if (state.lastSilenceRefusedMsg.length() > 0) {
    html += "<div class='refuse-banner'>⚠️ " + state.lastSilenceRefusedMsg + "</div>";
  }

  html += R"(<div class='card'>
  <div class='row'><span>Room State:</span><b id='r_state' style='color:#38bdf8'>)" + getDisplayStatus() + R"(</b></div>
  <div class='row'><span>Local Time (Dhaka):</span><b id='r_time'>)" + getDhakaTime() + R"(</b></div>
  <div class='row'><span>Connection State:</span><b id='r_conn' class=')" + String(state.wifiConnected ? "ok" : "warn") + R"('>)" + getConnectionState() + R"(</b></div>
  <div class='row'><span>Temperature:</span><b id='r_temp'>)" + String(state.temp, 1) + R"( °C</b></div>
  <div class='row'><span>Humidity:</span><b id='r_hum'>)" + String(state.hum, 1) + R"( %</b></div>
  <div class='row'><span>Heat Index:</span><b id='r_hi'>)" + String(state.heatIndex, 1) + R"( °C</b></div>
  <div class='row'><span>Heat Trend:</span><b id='r_trend'>)" + state.heatTrend + R"(</b></div>
  <div class='row'><span>Data Quality:</span><b id='r_qual' class=')" + String(state.quality == "good" ? "ok" : "warn") + R"('>)" + state.quality + R"(</b></div>
  <div class='row'><span>Flame Status:</span><span id='r_flame' class=')" + String(state.flame ? "warn" : "ok") + R"('>)" + String(state.flame ? "🔥 DETECTED" : "✅ SAFE") + R"(</span></div>
  <div class='row'><span>Door Status:</span><span id='r_door' class=')" + String(state.doorLocked ? "warn" : "ok") + R"('>)" + String(state.doorLocked ? "🔒 LOCKED" : "🔓 UNLOCKED") + R"(</span></div>
  <div class='row'><span>Last Card UID:</span><b id='r_card' style='color:#a855f7'>)" + state.lastCardUid + R"(</b></div>
  <div class='row'><span>WiFi RSSI:</span><b id='r_rssi'>)" + String(state.rssi) + R"( dBm</b></div>
  <div class='row'><span>Chip Temperature:</span><b id='r_chip'>)" + String(temperatureRead(), 1) + R"( °C</b></div>
  <div class='row'><span>Uptime:</span><b id='r_uptime'>)" + String(millis() / 1000) + R"( s</b></div>
  <div class='row'><span>Firmware:</span><b>v)" FIRMWARE_VERSION R"(</b></div>
  <div class='row'><span>Cloud URL:</span><small>)" + state.cloudUrl + R"(</small></div>
</div>

<div style='margin-bottom:1rem'>
  <form action='/door' method='POST' style='display:inline'>
    <button class='btn' type='submit'>Toggle Door</button>
  </form>
  <form action='/rfid/register_last' method='POST' style='display:inline;margin-left:.5rem'>
    <input type='hidden' name='role' value='student'>
    <button class='btn btn-green' type='submit'>Register Last Card as Student</button>
  </form>
  <form action='/rfid/register_last' method='POST' style='display:inline;margin-left:.5rem'>
    <input type='hidden' name='role' value='teacher'>
    <button class='btn btn-amber' type='submit'>Register Last Card as Teacher</button>
  </form>
  <a href='/stream' target='_blank' style='margin-left:.5rem'><button class='btn btn-blue' type='button'>Live Camera Feed</button></a>
</div>

<script>
  function update() {
    fetch('/status.json').then(r=>r.json()).then(d=>{
      document.getElementById('r_state').innerText = d.display_status;
      document.getElementById('r_time').innerText = d.local_time;
      document.getElementById('r_temp').innerText = d.temperature.toFixed(1) + ' °C';
      document.getElementById('r_hum').innerText = d.humidity.toFixed(1) + ' %';
      document.getElementById('r_hi').innerText = d.heat_index.toFixed(1) + ' °C';
      document.getElementById('r_trend').innerText = d.heat_trend;
      document.getElementById('r_qual').innerText = d.quality;
      document.getElementById('r_flame').innerText = d.flame ? '🔥 DETECTED' : '✅ SAFE';
      document.getElementById('r_flame').className = d.flame ? 'warn' : 'ok';
      document.getElementById('r_door').innerText = d.door_locked ? '🔒 LOCKED' : '🔓 UNLOCKED';
      document.getElementById('r_door').className = d.door_locked ? 'warn' : 'ok';
      document.getElementById('r_card').innerText = d.last_card_uid;
      document.getElementById('r_rssi').innerText = d.wifi_rssi + ' dBm';
      document.getElementById('r_chip').innerText = d.chip_temp.toFixed(1) + ' °C';
      document.getElementById('r_uptime').innerText = d.uptime_sec + ' s';
      document.getElementById('r_conn').innerText = d.connection_state;
      document.getElementById('r_conn').className = d.connection_state === 'LIVE' ? 'ok' : 'warn';
    }).catch(()=>{});
  }
  setInterval(update, 2000);
</script>
</body></html>)";

  server.send(200, "text/html", html);
}

static void handleStatusJson() {
  JsonDocument doc;
  doc["room_id"]          = state.roomId.isEmpty() ? "ROOM-01" : state.roomId;
  doc["room_state"]       = roomStateToString(state.roomState);
  doc["display_status"]   = getDisplayStatus();
  doc["local_time"]       = getDhakaTime();
  doc["temperature"]      = state.temp;
  doc["humidity"]         = state.hum;
  doc["heat_index"]       = state.heatIndex;
  doc["heat_trend"]       = state.heatTrend;
  doc["quality"]          = state.quality;
  doc["flame"]            = state.flame;
  doc["door_locked"]      = state.doorLocked;
  doc["door_status"]      = state.doorLocked ? "LOCKED" : "UNLOCKED";
  doc["last_card_uid"]    = state.lastCardUid;
  doc["wifi_rssi"]        = state.rssi;
  doc["chip_temp"]        = temperatureRead();
  doc["uptime_sec"]       = millis() / 1000;
  doc["firmware"]         = FIRMWARE_VERSION;
  doc["connection_state"] = getConnectionState();
  doc["silence_refused"]  = state.lastSilenceRefusedMsg;

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

static void handleRfidRegisterLast() {
  String role = server.hasArg("role") ? server.arg("role") : "student";
  rfidRegisterLast(role);
  server.sendHeader("Location", "/");
  server.send(303);
}

static void handleStream() {
  server.sendHeader("Location", "http://" + WiFi.localIP().toString() + ":" + String(CAMERA_STREAM_PORT) + "/stream");
  server.send(302);
}

// ── Public API ────────────────────────────────────────────

void localServerInit() {
  if (localServerRunning) return;
  if (!wifiIsConnected()) return; // Prevent port 80 collision with SmartProv in AP mode

  // Ensure LittleFS is mounted and /index.html is stored on flash filesystem (Level 2 Task 3)
  if (LittleFS.begin(true)) {
    if (!LittleFS.exists("/index.html")) {
      File f = LittleFS.open("/index.html", FILE_WRITE);
      if (f) {
        f.print("<!-- FastRobox Smart Classroom Local Status Page Stored in LittleFS -->");
        f.close();
      }
    }
  }

  server.on("/",                   HTTP_GET,  handleRoot);
  server.on("/status.json",        HTTP_GET,  handleStatusJson);
  server.on("/door",               HTTP_POST, handleDoorToggle);
  server.on("/rfid/register",      HTTP_POST, handleRfidRegister);
  server.on("/rfid/register_last", HTTP_POST, handleRfidRegisterLast);
  server.on("/stream",             HTTP_GET,  handleStream);
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
  Serial.printf("[http] Local server active at http://%s:%d\n",
                WiFi.localIP().toString().c_str(), LOCAL_SERVER_PORT);
}

void localServerHandle() {
  if (localServerRunning) {
    server.handleClient();
  }
}
