#pragma once
#include <Arduino.h>

void buzzerInit();
void buzzerBeep(int times, int ms);
void buzzerAlarm();   // continuous flame alarm pattern
void buzzerUpdate();
void buzzerSetSilenced(bool s);
bool buzzerIsSilenced();

