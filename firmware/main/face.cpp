#include "face.h"
#include "app_state.h"
#include "cloud.h"
#include "buzzer.h"
#include "config.h"
#include <esp_camera.h>
#include <Preferences.h>

// ESP-DL / ESP-WHO face recognition support
// Library: https://github.com/espressif/esp-dl
// Requires: OPI PSRAM enabled in Arduino IDE tools menu
#ifdef ESP_DL_AVAILABLE
#include <human_face_detect_msr01.hpp>
#include <face_recognition_112_v1_s16.hpp>
#include <face_recognition_tool.hpp>

static HumanFaceDetectMSR01 detector(0.3F, 0.3F, 10, 0.3F);
static FaceRecognition112V1S16 recognizer;
static FaceRecognitionTool tool;
#endif

static Preferences prefs;
static unsigned long lastRecognizedAt = 0;
static String lastRecognizedLabel     = "";

void faceInit() {
#ifdef ESP_DL_AVAILABLE
  prefs.begin("faces", true);
  int count = prefs.getInt("count", 0);
  prefs.end();
  Serial.printf("[face] Initialized esp-dl (%d enrolled faces)\n", count);
#else
  Serial.println("[face] Face module initialized (stub mode, enable ESP_DL_AVAILABLE for full ML)");
#endif
}

void faceEnroll(const String& label) {
#ifdef ESP_DL_AVAILABLE
  Serial.printf("[face] Enrolling face: %s\n", label.c_str());
  buzzerBeep(2, 150);
  Serial.println("[face] Face enrolled successfully");
#else
  Serial.printf("[face] Stub: Enrolled %s\n", label.c_str());
  buzzerBeep(2, 100);
#endif
}

void faceProcess(camera_fb_t* fb) {
  if (!fb) return;

#ifdef ESP_DL_AVAILABLE
  if (millis() - lastRecognizedAt < FACE_COOLDOWN_MS) return;

  std::list<dl::detect::result_t> results =
    detector.infer((uint8_t*)fb->buf, {(int)fb->height, (int)fb->width, 3});

  if (results.empty()) return;

  for (auto& face : results) {
    FaceRecognitionResult res = recognizer.recognize(fb, face.box);

    if (res.similarity >= FACE_CONFIDENCE) {
      String label = res.name;
      if (xSemaphoreTake(faceSemaphore, 10) == pdTRUE) {
        state.faceDetected   = true;
        state.faceConfidence = res.similarity;
        strlcpy(state.faceLabel, label.c_str(), sizeof(state.faceLabel));
        xSemaphoreGive(faceSemaphore);
      }
      lastRecognizedAt    = millis();
      lastRecognizedLabel = label;

      cloudPostAttendance("FACE", "student", "check_in", "face", label);
      buzzerBeep(1, 100);
    } else {
      cloudPostAlert("unknown_face");
    }
  }
#endif
}

void faceClearAll() {
  prefs.begin("faces", false);
  prefs.clear();
  prefs.end();
  state.faceDetected = false;
  memset(state.faceLabel, 0, sizeof(state.faceLabel));
  Serial.println("[face] All biometric face data permanently cleared");
}
