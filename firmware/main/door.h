#pragma once
#include <Arduino.h>

void doorInit();
void doorSetLocked(bool locked, const String& trigger);
void doorToggle(const String& trigger);
void doorUnlockTimed(uint32_t ms, const String& trigger);
void doorUnlockPermanent(const String& trigger);
void doorLock(const String& trigger);
void doorCheckAutoRelock();
void doorResetFire();
bool doorIsLocked();
