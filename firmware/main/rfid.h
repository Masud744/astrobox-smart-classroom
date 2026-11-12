#pragma once
#include <Arduino.h>

void rfidInit();
void rfidCheck();     // call every loop — scans for card, handles on detect
void rfidRegister(const String& uid, const String& role);  // save to NVS

