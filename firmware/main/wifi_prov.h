#pragma once
#include <Arduino.h>
#include "config.h"

// Define SmartProv customisation macros BEFORE including SmartProv.h
#define SP_AP_PREFIX  "SmartClass-10"
#define SP_LED_PIN    255        // Dummy pin to avoid collision with RFID_RST (GPIO 2)
#define SP_RESET_PIN  PIN_BOOT   // GPIO 0 BOOT button for factory reset (hold 3s)

#include <SmartProv.h>

extern SmartProv prov;

void wifiProvInit();
bool wifiIsConnected();
