#include "camera.h"
#include "app_state.h"
#include "face.h"
#include "config.h"
#include <esp_camera.h>
#include <WiFi.h>

// OV2640 pin configuration for ESP32-S3-CAM (Freenove / Standard N16R8 layout)
static camera_config_t camConfig = {
  .pin_pwdn     = -1,
  .pin_reset    = -1,
  .pin_xclk     = 15,
  .pin_sccb_sda = 4,
  .pin_sccb_scl = 5,
  .pin_d7       = 16,
  .pin_d6       = 17,
  .pin_d5       = 18,
  .pin_d4       = 12,
  .pin_d3       = 10,
  .pin_d2       = 8,
  .pin_d1       = 9,
  .pin_d0       = 11,
  .pin_vsync    = 6,
  .pin_href     = 7,
  .pin_pclk     = 13,
  .xclk_freq_hz = 20000000,
  .ledc_timer   = LEDC_TIMER_0,
  .ledc_channel = LEDC_CHANNEL_0,
  .pixel_format = PIXFORMAT_JPEG,
  .frame_size   = FRAMESIZE_SVGA,       // 800x600 resolution
  .jpeg_quality = 12,                   // Balance quality vs frame size
  .fb_count     = 2,                    // Double buffering in PSRAM
  .fb_location  = CAMERA_FB_IN_PSRAM,
  .grab_mode    = CAMERA_GRAB_WHEN_EMPTY,
};

static WiFiServer streamServer(CAMERA_STREAM_PORT);

void cameraInit() {
  esp_err_t err = esp_camera_init(&camConfig);
  if (err != ESP_OK) {
    Serial.printf("[cam] Camera init failed with error 0x%x\n", err);
    return;
  }
  Serial.println("[cam] OV2640 camera initialized (SVGA in PSRAM)");
}

// ── MJPEG Stream Handler ──────────────────────────────────

static void sendMjpegStream(WiFiClient& client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: multipart/x-mixed-replace; boundary=frame");
  client.println();

  while (client.connected()) {
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) continue;

    client.printf("--frame\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n", fb->len);
    client.write(fb->buf, fb->len);
    client.println();

    // Pass frame to face recognition pipeline
    if (xSemaphoreTake(faceSemaphore, 0) == pdTRUE) {
      faceProcess(fb);
      xSemaphoreGive(faceSemaphore);
    }

    esp_camera_fb_return(fb);
    delay(30);  // Cap stream to ~30 FPS
  }
}

// ── Core 1 FreeRTOS Camera Task ───────────────────────────

void cameraStreamTask(void* param) {
  streamServer.begin();
  Serial.printf("[cam] Stream server active: http://%s:%d/stream\n",
                WiFi.localIP().toString().c_str(), CAMERA_STREAM_PORT);

  while (true) {
    WiFiClient client = streamServer.accept();
    if (client) {
      sendMjpegStream(client);
      client.stop();
    } else {
      // Even without an active stream viewer, continuously process faces
      camera_fb_t* fb = esp_camera_fb_get();
      if (fb) {
        if (xSemaphoreTake(faceSemaphore, 0) == pdTRUE) {
          faceProcess(fb);
          xSemaphoreGive(faceSemaphore);
        }
        esp_camera_fb_return(fb);
      }
      vTaskDelay(pdMS_TO_TICKS(100));
    }
  }
}
