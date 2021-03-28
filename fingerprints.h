#ifndef FINGERPRINTS_H
#define FINGERPRINTS_H

#include "scan.h"
#include "utils.h"

namespace Fingerprints {

static const auto MAX_NETWORKS = 5;
static const auto MAX_TIMES_CAN_BE_SEEN = 10;
static const auto MIN_TIMES_CAN_BE_SEEN = 0;

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

storageStruct &load();
void save(storageStruct &fingerprints);
void update(storageStruct &fingerprints, wifiStruct *list);

}  // namespace Fingerprints
#endif