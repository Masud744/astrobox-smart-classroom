#pragma once
#include <Arduino.h>
#include <esp_camera.h>

void faceInit();
void faceEnroll(const String& label);   // save face embedding to NVS
void faceProcess(camera_fb_t* fb);      // detect + recognize in frame
void faceClearAll();                    // delete all embeddings (post-competition)

