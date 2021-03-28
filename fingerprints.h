#ifndef FINGERPRINTS_H
#define FINGERPRINTS_H

#include "scan.h"
#include "utils.h"

namespace Fingerprints {

static const auto MAX_NETWORKS = 5;
static const auto MAX_TIMES_CAN_BE_SEEN = 10;
static const auto MIN_TIMES_CAN_BE_SEEN = 2;

struct networkDataStruct {
  char SSID[MAX_LENGTH_SSID] = "\0";
  float rollingAverageRSSI = 0;
  float rollingVarianceRSSI = 0;
  uint8_t timesSeen = 0;  // times this network is seen
};

struct storageStruct {
  int state;
  networkDataStruct fingerprint[MAX_NETWORKS];
};

storageStruct &load(uint8_t positionNumber = 0);
void save(storageStruct &fingerprints, uint8_t positionNumber = 0);
float matchPercentage(storageStruct &fingerprints, wifiStruct *list);
void update(storageStruct &fingerprints, wifiStruct *list);
void print(storageStruct &fingerprints);

}  // namespace Fingerprints
#endif