#include "offline.h"
#include "app_state.h"
#include "wifi_prov.h"
#include <LittleFS.h>
#include <HTTPClient.h>

#define QUEUE_FILE "/offline_queue.jsonl"
#define MAX_ENTRIES 500

void offlineInit() {
  if (!LittleFS.begin(true)) {
    Serial.println("[offline] LittleFS mount failed!");
    return;
  }
  Serial.println("[offline] LittleFS storage initialized");
}

void offlineQueue(const String& path, const String& body) {
  File f = LittleFS.open(QUEUE_FILE, FILE_APPEND);
  if (!f) return;

  // Store entry as line: {"path":"/api/...","body":{...}}
  f.printf("{\"path\":\"%s\",\"body\":%s}\n", path.c_str(), body.c_str());
  f.close();

  Serial.printf("[offline] Queued offline event: %s\n", path.c_str());
}

void offlineFlush() {
  if (!LittleFS.exists(QUEUE_FILE)) return;
  if (!wifiIsConnected()) return;

  File f = LittleFS.open(QUEUE_FILE, FILE_READ);
  if (!f) return;

  int flushed = 0, failed = 0;

  while (f.available()) {
    String line = f.readStringUntil('\n');
    if (line.isEmpty()) continue;

    int pi = line.indexOf("\"path\":\"") + 8;
    int pe = line.indexOf("\"", pi);
    int bi = line.indexOf("\"body\":") + 7;

    String path = line.substring(pi, pe);
    String body = line.substring(bi, line.length() - 1);

    HTTPClient http;
    http.begin(state.cloudUrl + path);
    http.addHeader("Content-Type", "application/json");
    http.setTimeout(5000);
    int code = http.POST(body);
    http.end();

    if (code == 200 || code == 201) {
      flushed++;
    } else {
      failed++;
    }
  }
  f.close();

  if (failed == 0) {
    LittleFS.remove(QUEUE_FILE);
    Serial.printf("[offline] Successfully flushed %d offline records to cloud\n", flushed);
  } else {
    Serial.printf("[offline] Flushed %d records, %d failed (queue preserved)\n", flushed, failed);
  }
}
