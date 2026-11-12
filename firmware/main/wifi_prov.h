#pragma once
#include <Arduino.h>
#include <SmartProv.h>

extern SmartProv prov;

void wifiProvInit();
bool wifiIsConnected();
