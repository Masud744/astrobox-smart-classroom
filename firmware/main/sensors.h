#pragma once
#include <Arduino.h>

void sensorsInit();
void sensorsRead();      // updates state.temp, state.hum
void flameCheck();       // updates state.flame, triggers alert if new
void touchCheck();       // detects panic pad touch

