#ifndef UTILS_H
#define UTILS_H

#include "globals.h"

static const auto STATE_INITIALIZED = 127;
const auto SAMPLES = 100;
const auto MIN_ROLLING_VARIANCE = 5;

bool isRSSIOutsideNormal(int32_t rssi);
void updateRSSIStats(float rssi);
void blinkLight(const int color, uint32_t delayMillis);

#endif