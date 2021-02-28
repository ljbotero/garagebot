#include "globals.h"

bool isRSSIOutsideNormal(int32_t rssi);
void updateRSSIStats(float rssi);
void blinkLight(const int color, uint32_t delayMillis);