#pragma once
#include <Arduino.h>

void offlineInit();
void offlineQueue(const String& path, const String& body);
void offlineFlush();   // retry all queued requests — call after WiFi reconnect

