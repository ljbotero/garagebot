#include "utils.h"

#include "globals.h"

unsigned long wait_until_millis;
int continue_step_number = 0;

bool isWaiting() { return wait_until_millis > millis(); }

void wait(long waitMillis, int stepNumber) {
  if (!isWaiting()) {
    wait_until_millis = millis() + waitMillis;
    continue_step_number = stepNumber;
  }
}

void blinkLight(const int color, uint32_t delayMillis) {
  if (isWaiting()) {
    return;
  }
  analogWrite(RGB_R_PIN, 0);
  analogWrite(RGB_G_PIN, 0);
  analogWrite(RGB_B_PIN, 0);

  auto lightIntensity = map(analogRead(LDR_PIN), 40, 1023, 10, 255);
  if (continue_step_number == 0) {
    analogWrite(color, lightIntensity);
    wait(delayMillis, 1);
  } else if (continue_step_number == 1) {
    analogWrite(color, 0);
    wait(delayMillis, 2);
  } else {
    continue_step_number = 0;
  }
}