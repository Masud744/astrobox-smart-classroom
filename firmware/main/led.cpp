#include "led.h"
#include "config.h"

void ledInit() {
#ifdef RGB_PWR
  pinMode(RGB_PWR, OUTPUT);
  digitalWrite(RGB_PWR, HIGH);
#endif
  ledSet(0, 0, 0);
}

void ledSet(uint8_t r, uint8_t g, uint8_t b) {
  rgbLedWrite(PIN_RGB_LED, r, g, b);
}
