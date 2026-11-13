#include "rfid.h"
#include "app_state.h"
#include "cloud.h"
#include "door.h"
#include "buzzer.h"
#include "led.h"
#include "config.h"
#include <MFRC522.h>
#include <Preferences.h>
#include <SPI.h>

static MFRC522 mfrc(PIN_RFID_SDA, PIN_RFID_RST);
static Preferences prefs;
static unsigned long lastRfidCheckTime = 0;
static unsigned long rfidFailStartTime = 0;
static bool cardOnReader = false;

// ── Internal Helpers ──────────────────────────────────────

static String readUID() {
  String uid;
  for (byte i = 0; i < mfrc.uid.size; i++) {
    if (uid.length() > 0) uid += ":";
    if (mfrc.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(mfrc.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

static String lookupRole(const String& uid) {
  prefs.begin("rfid", true);
  String role = prefs.getString(uid.c_str(), "unknown");
  prefs.end();
  return role;
}

// ── Public API ────────────────────────────────────────────

void rfidInit() {
  SPI.begin(PIN_RFID_SCK, PIN_RFID_MISO, PIN_RFID_MOSI, PIN_RFID_SDA);
  mfrc.PCD_Init();
  byte ver = mfrc.PCD_ReadRegister(MFRC522::VersionReg);
  Serial.printf("[rfid] RC522 initialized (SCK:%d MISO:%d MOSI:%d SDA:%d RST:%d ver:0x%02X)\n",
                PIN_RFID_SCK, PIN_RFID_MISO, PIN_RFID_MOSI, PIN_RFID_SDA, PIN_RFID_RST, ver);
}

void rfidCheck() {
  unsigned long now = millis();

  // Periodic health check of RC522 SPI bus (3s timeout detection)
  if (now - lastRfidCheckTime >= 500) {
    lastRfidCheckTime = now;
    byte ver = mfrc.PCD_ReadRegister(MFRC522::VersionReg);

    // 0x00 or 0xFF indicates SPI bus disconnected or chip unresponsive
    if (ver == 0x00 || ver == 0xFF) {
      if (rfidFailStartTime == 0) rfidFailStartTime = now;
      if (now - rfidFailStartTime >= 3000 && !state.rfidError) {
        state.rfidError = true;
        Serial.println("[ERROR] SENSOR ERROR: RFID");
      }
    } else {
      if (state.rfidError) {
        state.rfidError = false;
        mfrc.PCD_Init(); // Re-arm chip registers on recovery
        Serial.println("[INFO] SENSOR RECOVERED: RFID");
      }
      rfidFailStartTime = 0;
    }
  }

  if (state.rfidError) return;

  // Level 1 (Row A1): Exact 3 lines for 3 taps, and 1 line for 5s hold.
  // When card is held, PICC_WakeupA detects presence and avoids duplicate [CARD] lines.
  if (cardOnReader) {
    byte bufferATQA[2];
    byte bufferSize = sizeof(bufferATQA);
    MFRC522::StatusCode status = mfrc.PICC_WakeupA(bufferATQA, &bufferSize);
    if (status != MFRC522::STATUS_OK) {
      // Card lifted from the reader
      cardOnReader = false;
      mfrc.PCD_StopCrypto1();
    } else {
      // Card is still being held continuously on the reader
      return;
    }
  }

  if (!mfrc.PICC_IsNewCardPresent() || !mfrc.PICC_ReadCardSerial()) return;

  cardOnReader = true;
  String uid = readUID();
  state.lastCardUid  = uid;
  state.lastCardTime = now;

  // Exact Level 1 required card log format
  Serial.printf("[CARD] uid=%s\n", uid.c_str());

  String role = lookupRole(uid);
  Serial.printf("[rfid] Card UID: %s | Role: %s\n", uid.c_str(), role.c_str());

  // Level 2: Send CARD_TAP event immediately to cloud
  cloudPostEvent("CARD_TAP", "{\"uid\":\"" + uid + "\",\"role\":\"" + role + "\"}");

  // Level 3 (Row A5): Fire alarm silence handling
  if (state.roomState == STATE_FIRE) {
    if (!state.flame && role == "teacher") {
      buzzerSetSilenced(true);
      state.lastSilenceRefusedMsg = "";
      buzzerBeep(2, 100);
      Serial.println("[EVENT] SILENCED");
      Serial.println("[flame] Teacher tag acknowledged: Alarm silenced. Door remains UNLOCKED.");
      cloudPostEvent("SILENCED", "{\"by\":\"teacher\"}");
    } else if (state.flame) {
      state.lastSilenceRefusedMsg = "CANNOT SILENCE – FLAME PRESENT";
      Serial.println("[flame] CANNOT SILENCE – FLAME PRESENT");
      buzzerAlarm();
    } else {
      Serial.println("[flame] Only Teacher tag can silence fire alarm!");
    }
    mfrc.PICC_HaltA();
    mfrc.PCD_StopCrypto1();
    return;
  }

  // Normal Access Control Rules:
  if (role == "teacher" || role == "student") {
    // Valid card entry:
    // Green light, 1 beep, exactly 5 seconds UNLOCKED, then automatically re-locks
    buzzerBeep(1, 100);
    ledSetOverride(true, 0, 255, 0); // Green LED
    doorUnlockTimed(5000, "rfid_" + role);
    cloudPostAttendance(uid, role, "check_in", "rfid", "");

    // Quick yield to keep green on for 300ms before returning to state LED
    vTaskDelay(pdMS_TO_TICKS(300));
    ledSetOverride(false);
  } else {
    // Invalid / Unregistered card:
    // Door NEVER unlocks, red light, 2 beeps, cloud alert
    buzzerBeep(2, 150);
    ledSetOverride(true, 255, 0, 0); // Red LED
    Serial.println("[rfid] Access Denied: Unregistered card. Door remains LOCKED.");
    cloudPostAlert("unknown_card");
    cloudPostAttendance(uid, "unknown", "denied", "rfid", "");

    vTaskDelay(pdMS_TO_TICKS(500));
    ledSetOverride(false);
  }

  mfrc.PICC_HaltA();
  mfrc.PCD_StopCrypto1();
}

void rfidRegister(const String& uid, const String& role) {
  prefs.begin("rfid", false);
  prefs.putString(uid.c_str(), role);
  prefs.end();
  Serial.printf("[rfid] Registered UID '%s' with role '%s'\n", uid.c_str(), role.c_str());
}

void rfidRegisterLast(const String& role) {
  if (state.lastCardUid.isEmpty() || state.lastCardUid == "-") {
    Serial.println("[rfid] No card scanned yet to register!");
    return;
  }
  rfidRegister(state.lastCardUid, role);
}
