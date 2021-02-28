#include "globals.h"

unsigned long wait_until_millis;
int continue_step_number = 0;

const auto SAMPLES = 100;
const auto MIN_ROLLING_VARIANCE = 5;
float rollingAverageRSSI = 0;
float rollingVarianceRSSI = 0;

bool isRSSIOutsideNormal(int32_t rssi) {
  auto upperLimit = rollingAverageRSSI + (rollingVarianceRSSI * 1.5);
  auto lowerLimit = rollingAverageRSSI - (rollingVarianceRSSI * 1.5);
  if (rssi > upperLimit || rssi < lowerLimit) {
    Serial.println(F(""));
    Serial.print(F("isRSSIOutsideNormal: "));
    Serial.print(lowerLimit);
    Serial.print(F(" < "));
    Serial.print(rssi);
    Serial.print(F(" < "));
    Serial.println(upperLimit);
    return true;
  }
  return false;
}

void updateRSSIStats(float rssi) {
  Serial.println(F(""));
  Serial.print(F("RSSI: "));
  Serial.print(rssi);

  if (rollingAverageRSSI == 0) {
    rollingAverageRSSI = rssi;
  }
  rollingAverageRSSI -= rollingAverageRSSI / SAMPLES;
  rollingAverageRSSI += rssi / SAMPLES;
  Serial.print(F(", rollingAverage: "));
  Serial.print(rollingAverageRSSI);

  float difference = abs(rssi - rollingAverageRSSI);
  if (rollingVarianceRSSI == 0) {
    rollingVarianceRSSI = MIN_ROLLING_VARIANCE;
  }
  if (difference > MIN_ROLLING_VARIANCE) {
    rollingVarianceRSSI -= rollingVarianceRSSI / SAMPLES;
    rollingVarianceRSSI += difference / SAMPLES;
  }
  Serial.print(F(", rollingVariance: "));
  Serial.println(rollingVarianceRSSI, 6);
}

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
