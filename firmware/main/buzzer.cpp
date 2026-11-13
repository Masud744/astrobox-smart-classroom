#include "buzzer.h"
#include "app_state.h"
#include "config.h"

static unsigned long lastDangerBeep = 0;
static bool fireSilenced = false;

void buzzerInit() {
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
  fireSilenced = false;
}

void buzzerSetSilenced(bool s) {
  fireSilenced = s;
  if (s) {
    digitalWrite(PIN_BUZZER, LOW);
  }
}

bool buzzerIsSilenced() {
  return fireSilenced;
}

void buzzerBeep(int times, int ms) {
  if (!state.buzzerOn) return;
  for (int i = 0; i < times; i++) {
    digitalWrite(PIN_BUZZER, HIGH);
    delay(ms);
    digitalWrite(PIN_BUZZER, LOW);
    if (i < times - 1) delay(ms);
  }
}

void buzzerAlarm() {
  fireSilenced = false;
  digitalWrite(PIN_BUZZER, HIGH);
}

void buzzerUpdate() {
  if (!state.buzzerOn) {
    digitalWrite(PIN_BUZZER, LOW);
    return;
  }

  unsigned long now = millis();

  if (state.roomState == STATE_FIRE) {
    if (!fireSilenced) {
      // Continuous buzzer during fire emergency
      digitalWrite(PIN_BUZZER, HIGH);
    } else {
      digitalWrite(PIN_BUZZER, LOW);
    }
  } else if (state.roomState == STATE_DANGER) {
    // Danger: short beep every 2 seconds
    if (now - lastDangerBeep >= 2000) {
      lastDangerBeep = now;
      digitalWrite(PIN_BUZZER, HIGH);
    } else if (now - lastDangerBeep >= 60) {
      digitalWrite(PIN_BUZZER, LOW);
    }
  } else if (state.roomState == STATE_PANIC) {
    // Panic buzzer pattern (150ms on, 150ms off)
    int cycle = (now / 150) % 2;
    digitalWrite(PIN_BUZZER, cycle == 0 ? HIGH : LOW);
  }
}
