#include "wifi_prov.h"
#include "app_state.h"
#include "config.h"
#include <WiFi.h>
#include <Preferences.h>

// Single global instance of SmartProv
SmartProv prov;
static Preferences apPrefs;
static String apPassword = "";

static String getApPassword() {
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
  // Register dynamic custom fields before calling begin()
  // These appear in the Captive Portal web UI and persist to NVS
  prov.addField("cloud_url",   "Cloud API URL",       "https://");
  prov.addField("room_id",     "Room ID",             "ROOM-01");
  prov.addField("class_start", "Class Start (HH:MM)", "08:00");

  // Initialise SmartProv using BOOT button (GPIO0) and dummy LED pin 255 (to protect GPIO2 RFID_RST)
  prov.begin(PIN_BOOT, 255);

  // Callback executed automatically as soon as Wi-Fi establishes connection
  prov.onConnected([]() {
    Serial.printf("\n[NET] Connected | IP: %s\n", prov.getIP().c_str());
    String url = prov.getField("cloud_url");
    if (url.length() > 0) state.cloudUrl = url;
    String rid = prov.getField("room_id");
    if (rid.length() > 0) state.roomId = rid;
    String cs = prov.getField("class_start");
    if (cs.length() > 0) state.classStart = cs;
    state.wifiConnected = true;

    Serial.printf("[SmartProv] IP: %s | RSSI: %d dBm\n", prov.getIP().c_str(), prov.getRSSI());
    Serial.printf("[cfg] Room: %s | Start: %s | Cloud: %s\n",
                  state.roomId.c_str(), state.classStart.c_str(), state.cloudUrl.c_str());
  });

  // Short non-blocking wait (up to 4s) if stored network credentials exist
  // If no credentials exist, prov.isSetupMode() is already true, so loop exits instantly
  if (!prov.isSetupMode()) {
    Serial.print("[wifi] Connecting to stored network");
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
    String cs = prov.getField("class_start");
    if (cs.length() > 0) state.classStart = cs;
    state.wifiConnected = true;
  } else if (prov.isSetupMode()) {
    String pass = getApPassword();
    Serial.printf("\n[NET] Starting AP SmartClass-T10, pass=%s at 192.168.4.1\n", pass.c_str());
    Serial.println("[SmartProv] Setup mode active. Connect phone to setup AP to configure.");
  } else {
    Serial.println("\n[wifi] Connecting in background...");
  }
}

bool wifiIsConnected() {
  return prov.isConnected() || (WiFi.status() == WL_CONNECTED);
}
