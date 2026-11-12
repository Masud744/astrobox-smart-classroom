#pragma once
#include <Arduino.h>

// ══════════════════════════════════════════════════════════
// PIN DEFINITIONS — ESP32-S3-CAM (Freenove / Standard N16R8)
// ══════════════════════════════════════════════════════════
// NOTE: Pins reserved by internal hardware (DO NOT USE FOR PERIPHERALS):
// - Camera OV2640: GPIO 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 16, 17, 18
// - Octal PSRAM (8MB): GPIO 35, 36, 37
// - UART0 (Flashing/Serial Monitor): GPIO 43 (TX), 44 (RX)
// - Native USB: GPIO 19, 20
// - BOOT Button: GPIO 0

// ── RC522 RFID Reader (SPI via ESP32-S3 GPIO Matrix) ──────
#define PIN_RFID_SCK    39
#define PIN_RFID_MISO   40
#define PIN_RFID_MOSI   41
#define PIN_RFID_SDA    42   // Chip Select (SS / SDA)
#define PIN_RFID_RST     2   // Reset

// ── External Sensors & Actuators ──────────────────────────
#define PIN_DHT         21   // DHT11 Data line
#define PIN_FLAME       47   // IR Flame Digital Output (Active LOW)
#define PIN_BUZZER      38   // Active Buzzer (Active HIGH)
#define PIN_TOUCH        1   // Touch Channel T1 (Jumper wire touch pad)

// ── Onboard Peripherals ───────────────────────────────────
#define PIN_BOOT         0   // BOOT Button (Active LOW)

#ifdef RGB_BUILTIN
  #define PIN_RGB_LED   RGB_BUILTIN
#else
  #define PIN_RGB_LED   48   // Standard ESP32-S3 onboard WS2812 LED
#endif

// ── Timing Intervals (ms) ─────────────────────────────────
#define INTERVAL_SENSOR       5000   // 5s: Read sensors & POST to cloud
#define INTERVAL_SETTINGS    10000   // 10s: Poll remote cloud settings
#define INTERVAL_HEARTBEAT   30000   // 30s: System heartbeat
#define INTERVAL_OTA_CHECK 1800000   // 30m: Check for new OTA firmware

// ── Thresholds & Logic ────────────────────────────────────
#define FACE_CONFIDENCE       0.75f  // Minimum score for face recognition
#define FACE_COOLDOWN_MS     10000   // Minimum ms between same-face checkins
#define TOUCH_DELTA_TRIGGER  15000   // Delta above baseline on ESP32-S3 touch

// ── System ────────────────────────────────────────────────
#define FIRMWARE_VERSION     "1.0.0"
#define LOCAL_SERVER_PORT    80
#define CAMERA_STREAM_PORT   81
