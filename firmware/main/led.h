#pragma once
#include <Arduino.h>

void ledInit();
void ledSet(uint8_t r, uint8_t g, uint8_t b);
void ledStartHeartbeat();
void ledSetOverride(bool active, uint8_t r = 0, uint8_t g = 0, uint8_t b = 0);


