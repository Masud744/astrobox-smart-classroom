#include <Arduino.h>
#include <SmartProv.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WebServer.h>
#include <DHT.h>
#include <MFRC522.h>
#include <SPI.h>
#include <ArduinoJson.h>
#include <time.h>
#include "config.h"

// ── Global state ───────────────────────────────────────────
struct Settings {
  String roomId       = "ROOM-01";
  String classStart   = "08:00";
  float  tempMax      = 35.0;
  float  humMax       = 80.0;
  bool   buzzerOn     = true;
  bool   doorLocked   = true;
};

struct SensorData {
  float temp    = 0;
  float hum     = 0;
  bool  flame   = false;
  int   rssi    = 0;
};

Settings   cfg;
SensorData sensor;

String cloudUrl;   // set by SmartProv custom field
bool   offlineMode = false;

// ── Drivers ────────────────────────────────────────────────
DHT     dht(PIN_DHT, DHT11);
MFRC522 rfid(PIN_RFID_SDA, PIN_RFID_RST);
WebServer localServer(80);

// ── Timers ─────────────────────────────────────────────────
unsigned long tSensor    = 0;
unsigned long tSettings  = 0;
unsigned long tHeartbeat = 0;

// ── Prototypes ─────────────────────────────────────────────
void setupPins();
void setupSmartProv();
void setupLocalServer();
void setupNTP();
void readSensors();
void checkFlame();
void checkTouch();
void checkRFID();
void pollSettings();
void postSensor();
void postAlert(String type);
void postAttendance(String uid, String personType, String action, String method);
void postDoor(String state, String trigger);
void postHeartbeat();
void setDoor(bool locked, String trigger);
void setLED(int r, int g, int b);
void beep(int times, int ms);
bool httpPost(String path, String body);
String httpGet(String path);

// ══════════════════════════════════════════════════════════
void setup() {
  Serial.begin(115200);
  Serial.println("[boot] FastRobox Smart Classroom v" FIRMWARE_VERSION);

  setupPins();
  setupSmartProv();   // blocks until WiFi connected via portal
  setupNTP();
  setupLocalServer();

  dht.begin();
  SPI.begin(PIN_RFID_SCK, PIN_RFID_MISO, PIN_RFID_MOSI, PIN_RFID_SDA);
  rfid.PCD_Init();

  setLED(0, 255, 0);  // green = ready
  beep(1, 100);
  Serial.println("[ready] All systems up");
  pollSettings();
  postHeartbeat();
}

void loop() {
  SmartProv.handle();
  localServer.handleClient();

  unsigned long now = millis();

  if (now - tSensor >= INTERVAL_SENSOR) {
    tSensor = now;
    readSensors();
    checkFlame();
    postSensor();
  }

  if (now - tSettings >= INTERVAL_SETTINGS) {
    tSettings = now;
    pollSettings();
  }

  if (now - tHeartbeat >= INTERVAL_HEARTBEAT) {
    tHeartbeat = now;
    postHeartbeat();
  }

  checkTouch();
  checkRFID();
}

// ══════════════════════════════════════════════════════════

void setupPins() {
  pinMode(PIN_FLAME,  INPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_TOUCH,  INPUT);
  pinMode(PIN_BOOT,   INPUT_PULLUP);
  pinMode(PIN_LED_R,  OUTPUT);
  pinMode(PIN_LED_G,  OUTPUT);
  pinMode(PIN_LED_B,  OUTPUT);
  setLED(255, 165, 0);  // orange = booting
}

void setupSmartProv() {
  // Custom fields: cloud URL, Room ID
  SmartProv.addField("cloud_url",   "Cloud API URL",   "https://");
  SmartProv.addField("room_id",     "Room ID",         "ROOM-01");
  SmartProv.addField("class_start", "Class Start Time","08:00");

  SmartProv.begin("FastRobox-Setup");

  while (!SmartProv.isConnected()) {
    SmartProv.handle();
    delay(100);
  }

  cloudUrl        = SmartProv.getField("cloud_url");
  cfg.roomId      = SmartProv.getField("room_id");
  cfg.classStart  = SmartProv.getField("class_start");

  Serial.println("[wifi] Connected: " + WiFi.localIP().toString());
  Serial.println("[cfg]  Room: " + cfg.roomId + " | Cloud: " + cloudUrl);
}

void setupNTP() {
  configTime(6 * 3600, 0, "pool.ntp.org");
  Serial.print("[ntp]  Syncing");
  struct tm t;
  for (int i = 0; i < 20 && !getLocalTime(&t); i++) { delay(500); Serial.print("."); }
  Serial.println(" done");
}

void setupLocalServer() {
  localServer.on("/", []() {
    String html = "<html><head><meta http-equiv='refresh' content='3'><style>body{font-family:sans-serif;background:#111;color:#eee;padding:2rem}</style></head><body>";
    html += "<h2>FastRobox — " + cfg.roomId + "</h2>";
    html += "<p>Temp: <b>" + String(sensor.temp, 1) + "°C</b> | Hum: <b>" + String(sensor.hum, 1) + "%</b></p>";
    html += "<p>Flame: <b>" + String(sensor.flame ? "🔥 DETECTED" : "✅ SAFE") + "</b></p>";
    html += "<p>Door: <b>" + String(cfg.doorLocked ? "🔒 LOCKED" : "🔓 UNLOCKED") + "</b></p>";
    html += "<p>WiFi RSSI: <b>" + String(sensor.rssi) + " dBm</b></p>";
    html += "<p>Firmware: <b>v" FIRMWARE_VERSION "</b></p>";
    html += "</body></html>";
    localServer.send(200, "text/html", html);
  });

  localServer.on("/status.json", []() {
    String json = "{\"room\":\"" + cfg.roomId + "\",\"temp\":" + sensor.temp +
                  ",\"hum\":" + sensor.hum + ",\"flame\":" + (sensor.flame?"true":"false") +
                  ",\"door_locked\":" + (cfg.doorLocked?"true":"false") + "}";
    localServer.send(200, "application/json", json);
  });

  localServer.begin();
  Serial.println("[http] Local server: http://" + WiFi.localIP().toString());
}

// ── Sensor reads ───────────────────────────────────────────
void readSensors() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  if (!isnan(t)) sensor.temp = t;
  if (!isnan(h)) sensor.hum  = h;
  sensor.rssi = WiFi.RSSI();
  Serial.printf("[sensor] %.1f°C  %.1f%%  RSSI:%d\n", sensor.temp, sensor.hum, sensor.rssi);
}

void checkFlame() {
  bool flame = !digitalRead(PIN_FLAME);  // active LOW
  if (flame && !sensor.flame) {
    // new flame event
    setLED(255, 0, 0);
    beep(3, 200);
    postAlert("flame");
    Serial.println("[alert] FLAME DETECTED");
  }
  sensor.flame = flame;
  if (!flame) setLED(0, 255, 0);
}

void checkTouch() {
  static bool lastTouch = false;
  bool t = touchRead(PIN_TOUCH) < 40;
  if (t && !lastTouch) {
    Serial.println("[panic] Touch pad triggered");
    postAlert("panic");
    beep(5, 100);
  }
  lastTouch = t;
}

void checkRFID() {
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) return;

  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (uid.length()) uid += ":";
    uid += String(rfid.uid.uidByte[i] < 0x10 ? "0" : "") + String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  Serial.println("[rfid] Card: " + uid);

  // UID-to-role mapping stored in NVS (set via SmartProv or dashboard)
  // For now: white card = student, blue tag = teacher, else unknown
  // This will be configured on competition day
  String role = "unknown";
  String action = "denied";

  // TODO: load registered UIDs from NVS after competition setup
  beep(role == "unknown" ? 2 : 1, 150);
  postAttendance(uid, role, action, "rfid");

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// ── Cloud comms ────────────────────────────────────────────
void pollSettings() {
  String res = httpGet("/api/settings");
  if (res.isEmpty()) return;
  JsonDocument doc;
  if (deserializeJson(doc, res)) return;
  cfg.roomId      = doc["room_id"]       | cfg.roomId;
  cfg.classStart  = doc["class_start"]   | cfg.classStart;
  cfg.tempMax     = doc["temp_max"]      | cfg.tempMax;
  cfg.humMax      = doc["hum_max"]       | cfg.humMax;
  cfg.buzzerOn    = doc["buzzer_enabled"] | cfg.buzzerOn;

  bool newDoor = doc["door_locked"] | cfg.doorLocked;
  if (newDoor != cfg.doorLocked) setDoor(newDoor, "dashboard");

  Serial.printf("[cfg]  tempMax=%.1f door=%s\n", cfg.tempMax, cfg.doorLocked?"locked":"unlocked");
}

void postSensor() {
  String body = "{\"room_id\":\"" + cfg.roomId + "\","
                "\"temperature\":" + String(sensor.temp, 1) + ","
                "\"humidity\":"    + String(sensor.hum, 1)  + ","
                "\"flame\":"       + (sensor.flame?"true":"false") + ","
                "\"wifi_rssi\":"   + String(sensor.rssi) + "}";
  httpPost("/api/sensor", body);
}

void postAlert(String type) {
  String body = "{\"room_id\":\"" + cfg.roomId + "\",\"alert_type\":\"" + type + "\"}";
  httpPost("/api/alert", body);
}

void postAttendance(String uid, String personType, String action, String method) {
  String body = "{\"room_id\":\"" + cfg.roomId + "\","
                "\"card_uid\":\"" + uid + "\","
                "\"person_type\":\"" + personType + "\","
                "\"action\":\"" + action + "\","
                "\"method\":\"" + method + "\"}";
  httpPost("/api/attendance", body);
}

void postDoor(String state, String trigger) {
  String body = "{\"room_id\":\"" + cfg.roomId + "\","
                "\"state\":\"" + state + "\","
                "\"trigger\":\"" + trigger + "\"}";
  httpPost("/api/door", body);
}

void postHeartbeat() {
  String body = "{\"room_id\":\"" + cfg.roomId + "\","
                "\"firmware_version\":\"" FIRMWARE_VERSION "\","
                "\"ip_address\":\"" + WiFi.localIP().toString() + "\","
                "\"uptime_sec\":" + String(millis()/1000) + "}";
  httpPost("/api/heartbeat", body);
}

void setDoor(bool locked, String trigger) {
  cfg.doorLocked = locked;
  setLED(locked ? 255 : 0, locked ? 0 : 255, 0);
  postDoor(locked ? "locked" : "unlocked", trigger);
  Serial.println("[door] " + String(locked ? "LOCKED" : "UNLOCKED") + " by " + trigger);
}

// ── Helpers ────────────────────────────────────────────────
bool httpPost(String path, String body) {
  if (cloudUrl.isEmpty() || WiFi.status() != WL_CONNECTED) {
    offlineMode = true;
    // TODO: queue to SPIFFS for later flush
    return false;
  }
  HTTPClient http;
  http.begin(cloudUrl + path);
  http.addHeader("Content-Type", "application/json");
  int code = http.POST(body);
  http.end();
  offlineMode = false;
  return code == 200;
}

String httpGet(String path) {
  if (cloudUrl.isEmpty() || WiFi.status() != WL_CONNECTED) return "";
  HTTPClient http;
  http.begin(cloudUrl + path);
  int code = http.GET();
  if (code != 200) { http.end(); return ""; }
  String res = http.getString();
  http.end();
  return res;
}

void setLED(int r, int g, int b) {
  analogWrite(PIN_LED_R, r);
  analogWrite(PIN_LED_G, g);
  analogWrite(PIN_LED_B, b);
}

void beep(int times, int ms) {
  if (!cfg.buzzerOn) return;
  for (int i = 0; i < times; i++) {
    digitalWrite(PIN_BUZZER, HIGH); delay(ms);
    digitalWrite(PIN_BUZZER, LOW);  delay(ms);
  }
}
