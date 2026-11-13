#include "wifi_prov.h"
#include "app_state.h"
#include "config.h"
#include <WiFi.h>

// Single global instance of SmartProv
SmartProv prov;

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
    Serial.println("\n[SmartProv] Wi-Fi connection established!");
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
    Serial.println("\n[SmartProv] Setup mode active. Connect phone to setup AP to configure.");
  } else {
    Serial.println("\n[wifi] Connecting in background...");
  }
}

bool wifiIsConnected() {
  return prov.isConnected() || (WiFi.status() == WL_CONNECTED);
}
