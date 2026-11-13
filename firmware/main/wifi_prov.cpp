#include "wifi_prov.h"
#include "app_state.h"
#include "config.h"
#include <WiFi.h>
#include <esp_task_wdt.h>

void wifiProvInit() {
  // Dynamic fields configured via captive portal — NO HARDCODED VALUES
  prov.addField("cloud_url",   "Cloud API URL",       "https://");
  prov.addField("room_id",     "Room ID",             "ROOM-01");
  prov.addField("class_start", "Class Start (HH:MM)", "08:00");

  prov.begin(PIN_BOOT, PIN_RGB_LED);

  Serial.print("[wifi] Waiting for network connection");
  while (!prov.isConnected()) {
    prov.update();
    esp_task_wdt_reset();
    Serial.print(".");
    delay(200);
  }
  Serial.println(" connected!");

  state.cloudUrl      = prov.getField("cloud_url");
  state.roomId        = prov.getField("room_id");
  state.classStart    = prov.getField("class_start");
  state.wifiConnected = true;

  Serial.printf("[wifi] IP: %s | RSSI: %d dBm\n",
                prov.getIP().c_str(), prov.getRSSI());
  Serial.printf("[cfg]  Room: %s | Start: %s | Cloud: %s\n",
                state.roomId.c_str(), state.classStart.c_str(), state.cloudUrl.c_str());
}

bool wifiIsConnected() {
  return prov.isConnected() || (WiFi.status() == WL_CONNECTED);
}
