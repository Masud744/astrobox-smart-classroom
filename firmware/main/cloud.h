#pragma once
#include <Arduino.h>

bool cloudPostTelemetry();
bool cloudPostEvent(const String& type, const String& detailJson = "{}");
bool cloudPostSensor();
bool cloudPostAttendance(const String& uid, const String& role,
                         const String& action, const String& method,
                         const String& faceLabel = "");
bool cloudPostDoor(const String& state, const String& trigger);
bool cloudPostAlert(const String& type);
bool cloudPostHeartbeat();
bool cloudPollSettings();   // returns true if settings changed
bool cloudCheckOTA();       // returns true if OTA triggered

