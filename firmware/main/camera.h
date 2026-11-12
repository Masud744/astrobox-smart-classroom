#pragma once
#include <Arduino.h>

void cameraInit();
void cameraStreamTask(void* param);  // FreeRTOS task — runs on Core 1

