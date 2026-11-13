#include "door.h"
#include "app_state.h"
#include "cloud.h"
#include "led.h"

void doorInit() {
  state.doorLocked = true;
  Serial.println("[door] State initialized: LOCKED");
}

void doorSetLocked(bool locked, const String& trigger) {
  if (state.doorLocked == locked) return;

  state.doorLocked = locked;
  Serial.printf("[door] %s triggered by %s\n",
                locked ? "LOCKED" : "UNLOCKED", trigger.c_str());

  cloudPostDoor(locked ? "locked" : "unlocked", trigger);
}

void doorToggle(const String& trigger) {
  doorSetLocked(!state.doorLocked, trigger);
}

bool doorIsLocked() {
  return state.doorLocked;
}
