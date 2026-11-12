#pragma once
#include <Arduino.h>

void doorInit();
void doorSetLocked(bool locked, const String& trigger);
void doorToggle(const String& trigger);
bool doorIsLocked();

