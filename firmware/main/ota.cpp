#include "ota.h"
#include <HTTPUpdate.h>
#include <WiFiClientSecure.h>

void otaFlash(const String& url) {
  Serial.printf("[ota] Flashing from: %s\n", url.c_str());

  WiFiClientSecure client;
  client.setInsecure();  // skip cert verify for competition

  httpUpdate.setLedPin(LED_BUILTIN, LOW);
  httpUpdate.onStart([]()    { Serial.println("[ota] Starting..."); });
  httpUpdate.onEnd([]()      { Serial.println("[ota] Done! Rebooting..."); });
  httpUpdate.onError([](int e){ Serial.printf("[ota] Error: %d\n", e); });
  httpUpdate.onProgress([](int cur, int total) {
    Serial.printf("[ota] %d / %d bytes\n", cur, total);
  });

  t_httpUpdate_return ret = httpUpdate.update(client, url);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("[ota] Failed: %s\n", httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("[ota] No update needed");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("[ota] Success — restarting");
      ESP.restart();
      break;
  }
}

