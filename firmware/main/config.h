#pragma once

// ── Pin Definitions ────────────────────────────────────────
#define PIN_DHT       4
#define PIN_FLAME     5
#define PIN_BUZZER    6
#define PIN_TOUCH     1    // jumper wire touch pad (panic)

// RC522 RFID (SPI)
#define PIN_RFID_SDA  10
#define PIN_RFID_SCK  12
#define PIN_RFID_MOSI 11
#define PIN_RFID_MISO 13
#define PIN_RFID_RST  9

// Onboard RGB LED (ESP32-S3-CAM)
#define PIN_LED_R     47
#define PIN_LED_G     48
#define PIN_LED_B     21

// BOOT button
#define PIN_BOOT      0

// ── Intervals (ms) ─────────────────────────────────────────
#define INTERVAL_SENSOR     5000   // read + post sensor data
#define INTERVAL_SETTINGS   10000  // poll remote settings
#define INTERVAL_HEARTBEAT  30000  // device heartbeat

// ── Firmware Info ──────────────────────────────────────────
#define FIRMWARE_VERSION "1.0.0"
