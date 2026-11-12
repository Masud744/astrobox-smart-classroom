#include "buzzer.h"
#include "app_state.h"
#include "config.h"

void buzzerInit() {
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, LOW);
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
  // Urgent 3-beep alarm pattern for flame or panic condition
  buzzerBeep(3, 200);
}
