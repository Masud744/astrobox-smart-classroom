#include "rfid.h"
#include "app_state.h"
#include "cloud.h"
#include "door.h"
#include "buzzer.h"
#include "config.h"
#include <MFRC522.h>
#include <Preferences.h>
#include <SPI.h>

static MFRC522 mfrc(PIN_RFID_SDA, PIN_RFID_RST);
static Preferences prefs;

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
  Serial.printf("[rfid] RC522 initialized (SCK:%d MISO:%d MOSI:%d SDA:%d RST:%d)\n",
                PIN_RFID_SCK, PIN_RFID_MISO, PIN_RFID_MOSI, PIN_RFID_SDA, PIN_RFID_RST);
}

void rfidCheck() {
  if (!mfrc.PICC_IsNewCardPresent() || !mfrc.PICC_ReadCardSerial()) return;

  String uid  = readUID();
  String role = lookupRole(uid);
  Serial.printf("[rfid] Card UID: %s | Role: %s\n", uid.c_str(), role.c_str());

  String action = "denied";

  if (role == "teacher") {
    action = "check_in";
    doorToggle("rfid_teacher");
    buzzerBeep(1, 100);
  } else if (role == "student") {
    action = "check_in";
    buzzerBeep(1, 100);
  } else {
    // Unknown or unregistered card
    buzzerBeep(2, 150);
    cloudPostAlert("unknown_card");
  }

  cloudPostAttendance(uid, role, action, "rfid", "");

  mfrc.PICC_HaltA();
  mfrc.PCD_StopCrypto1();
}

void rfidRegister(const String& uid, const String& role) {
  prefs.begin("rfid", false);
  prefs.putString(uid.c_str(), role);
  prefs.end();
  Serial.printf("[rfid] Registered UID '%s' with role '%s'\n", uid.c_str(), role.c_str());
}
