#include "wifi_prov.h"
#include "app_state.h"
#include "config.h"
#include <WiFi.h>
#include <Preferences.h>

// Single global instance of SmartProv
SmartProv prov;
static Preferences apPrefs;
static String apPassword = "";

String wifiGetApPassword() {
  if (apPassword.length() >= 8) return apPassword;
  apPrefs.begin("wifi_ap", false);
  apPassword = apPrefs.getString("pass", "");
  if (apPassword.length() < 8) {
    const char chars[] = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghjkmnpqrstuvwxyz23456789";
    apPassword = "";
    for (int i = 0; i < 8; i++) {
      apPassword += chars[random(0, sizeof(chars) - 1)];
    }
    apPrefs.putString("pass", apPassword);
  }
  apPrefs.end();
  return apPassword;
}

void wifiProvInit() {
  String pass = wifiGetApPassword();
  prov.setAPPassword(pass);

  // Register dynamic custom fields before calling begin() (Level 2 Task 2)
  // These appear in the Captive Portal web UI and persist to NVS
  prov.addField("room_id",      "Room ID",             "ROOM-01");
  prov.addField("device_token", "Device Token",        "token_dev_10");
  prov.addField("cloud_url",    "Cloud API URL",       "https://");
  prov.addField("class_start",  "Class Start (HH:MM)", "08:00");

  // Initialise SmartProv using BOOT button (GPIO0) and dummy LED pin 255 (to protect GPIO2 RFID_RST)
  prov.begin(PIN_BOOT, 255);

  // Callback executed automatically as soon as Wi-Fi establishes connection
  prov.onConnected([]() {
    Serial.printf("\n[NET] Connected | IP: %s\n", prov.getIP().c_str());
    String url = prov.getField("cloud_url");
    if (url.length() > 0) state.cloudUrl = url;
    String rid = prov.getField("room_id");
    if (rid.length() > 0) state.roomId = rid;
    String token = prov.getField("device_token");
    if (token.length() > 0) state.deviceToken = token;
    String cs = prov.getField("class_start");
    if (cs.length() > 0) state.classStart = cs;
    state.wifiConnected = true;

    Serial.printf("[NET] IP: %s | RSSI: %d dBm\n", prov.getIP().c_str(), prov.getRSSI());
    Serial.printf("[cfg] Room: %s | Token: %s | Cloud: %s\n",
                  state.roomId.c_str(), state.deviceToken.c_str(), state.cloudUrl.c_str());
  });

  // Short non-blocking wait (up to 4s) if stored network credentials exist
  // If no credentials exist, prov.isSetupMode() is already true, so loop exits instantly
  if (!prov.isSetupMode()) {
    Serial.print("[NET] Connecting to stored network");
    unsigned long start = millis();
    while (!prov.isConnected() && !prov.isSetupMode() && (millis() - start < 4000)) {
      prov.update();
      Serial.print(".");
      delay(200);
    }
  }

  if (prov.isConnected()) {
    Serial.println(" connected!");
    String url = prov.getField("cloud_url");
    if (url.length() > 0) state.cloudUrl = url;
    String rid = prov.getField("room_id");
    if (rid.length() > 0) state.roomId = rid;
    String token = prov.getField("device_token");
    if (token.length() > 0) state.deviceToken = token;
    String cs = prov.getField("class_start");
    if (cs.length() > 0) state.classStart = cs;
    state.wifiConnected = true;
  } else if (prov.isSetupMode()) {
    String pass = wifiGetApPassword();
    WiFi.softAP("SmartClass-T10", pass.c_str());
    Serial.printf("\n[NET] Starting AP SmartClass-T10, pass=%s at 192.168.4.1\n", pass.c_str());
    Serial.println("[SmartProv] Setup mode active. Connect phone to setup AP to configure.");
  } else {
    Serial.println("\n[NET] Connecting in background...");
  }
}

void wifiProvLoop() {
  prov.update();
  static bool apActive = false;
  if (prov.isSetupMode()) {
    if (!apActive || WiFi.softAPSSID() != "SmartClass-T10") {
      String pass = wifiGetApPassword();
      WiFi.softAP("SmartClass-T10", pass.c_str());
      apActive = true;
    }
  } else {
    apActive = false;
  }
}

bool wifiIsConnected() {
  return prov.isConnected() || (WiFi.status() == WL_CONNECTED);
}
