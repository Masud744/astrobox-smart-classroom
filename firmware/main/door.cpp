#include "door.h"
#include "app_state.h"
#include "cloud.h"
#include "config.h"
#include <Preferences.h>

static Preferences doorPrefs;
static unsigned long autoRelockTime = 0;
static bool isPermanentUnlock = false;

void doorInit() {
  pinMode(PIN_FLAME, INPUT);

  // Requirement: If flame is present right at the moment of boot, device boots UNLOCKED
  if (digitalRead(PIN_FLAME) == LOW) {
    state.doorLocked = false;
    isPermanentUnlock = true;
    state.roomState = STATE_FIRE;
    Serial.println("[door] Flame detected at boot! Door initialized: UNLOCKED");
    return;
  }

  // Otherwise load persisted state from NVS (persists across power cuts during fire)
  doorPrefs.begin("door_state", true);
  state.doorLocked = doorPrefs.getBool("locked", true);
  bool fireSaved = doorPrefs.getBool("fire", false);
  doorPrefs.end();

  if (fireSaved) {
    state.doorLocked = false;
    isPermanentUnlock = true;
    state.roomState = STATE_FIRE;
    Serial.println("[door] Restored FIRE state from flash: Door initialized: UNLOCKED");
  } else {
    Serial.printf("[door] State initialized from flash: %s\n",
                  state.doorLocked ? "LOCKED" : "UNLOCKED");
  }
}

void doorSetLocked(bool locked, const String& trigger) {
  state.doorLocked = locked;
  if (locked) {
    autoRelockTime = 0;
    isPermanentUnlock = false;
  }

  // Persist state to flash
  doorPrefs.begin("door_state", false);
  doorPrefs.putBool("locked", locked);
  doorPrefs.end();

  Serial.printf("[door] %s triggered by %s\n",
                locked ? "LOCKED" : "UNLOCKED", trigger.c_str());

  cloudPostDoor(locked ? "locked" : "unlocked", trigger);
}

void doorToggle(const String& trigger) {
  doorSetLocked(!state.doorLocked, trigger);
}

void doorUnlockTimed(uint32_t ms, const String& trigger) {
  state.doorLocked = false;
  isPermanentUnlock = false;
  autoRelockTime = millis() + ms;

  Serial.printf("[door] UNLOCKED for %u ms triggered by %s\n", ms, trigger.c_str());
  cloudPostDoor("unlocked", trigger);
}

void doorUnlockPermanent(const String& trigger) {
  state.doorLocked = false;
  isPermanentUnlock = true;
  autoRelockTime = 0;

  // Persist permanent unlocked state (e.g. fire emergency)
  doorPrefs.begin("door_state", false);
  doorPrefs.putBool("locked", false);
  if (trigger == "flame") {
    doorPrefs.putBool("fire", true);
  }
  doorPrefs.end();

  Serial.printf("[door] PERMANENT UNLOCKED triggered by %s\n", trigger.c_str());
  cloudPostDoor("unlocked", trigger);
}

void doorLock(const String& trigger) {
  doorSetLocked(true, trigger);
}

void doorCheckAutoRelock() {
  if (!state.doorLocked && !isPermanentUnlock && autoRelockTime > 0) {
    if (millis() >= autoRelockTime) {
      autoRelockTime = 0;
      doorLock("auto_relock");
    }
  }
}

void doorResetFire() {
  isPermanentUnlock = false;
  autoRelockTime = 0;
  state.roomState = STATE_SAFE;

  doorPrefs.begin("door_state", false);
  doorPrefs.putBool("locked", true);
  doorPrefs.putBool("fire", false);
  doorPrefs.end();

  doorLock("reset_fire");
  Serial.println("[fire] Reset fire executed: Door LOCKED, status SAFE");
}

bool doorIsLocked() {
  return state.doorLocked;
}
