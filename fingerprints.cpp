#include "fingerprints.h"

#include <EEPROM.h>

#include "scan.h"

namespace Fingerprints {

storageStruct &load(uint8_t positionNumber) {
  storageStruct fingerprints;
  EEPROM.begin(sizeof(fingerprints));
  int address = positionNumber * sizeof(struct storageStruct);
  EEPROM.get(address, fingerprints);
  if (fingerprints.state != STATE_INITIALIZED) {
    storageStruct newData;
    newData.state = STATE_INITIALIZED;
    EEPROM.put(address, newData);
    EEPROM.commit();
    fingerprints = newData;
    Serial.println(F("EEPROM Initialized"));
  }
  return fingerprints;
}

void save(storageStruct &fingerprints, uint8_t positionNumber) {
  int address = positionNumber * sizeof(struct storageStruct);
  EEPROM.put(address, fingerprints);
  EEPROM.commit();
}

void print(storageStruct &fingerprints) {
  for (auto fingerprintNum = 0; fingerprintNum < MAX_NETWORKS;
       fingerprintNum++) {
    networkDataStruct &fingerprint = fingerprints.fingerprint[fingerprintNum];
    Serial.print(F("Fingerprint: "));
    Serial.print(fingerprint.SSID);
    Serial.print(F(" ("));
    Serial.print(fingerprint.rollingAverageRSSI);
    Serial.print(F("db +-"));
    Serial.print(fingerprint.rollingVarianceRSSI);
    Serial.print(F(") seen "));
    Serial.print(fingerprint.timesSeen);
    Serial.println(F(" times"));
  }
}

bool isSSIDInFingerprints(storageStruct &fingerprints, const String &ssid) {
  for (auto fingerprintNum = 0; fingerprintNum < MAX_NETWORKS;
       fingerprintNum++) {
    networkDataStruct &fingerprint = fingerprints.fingerprint[fingerprintNum];
    if (strncmp(fingerprint.SSID, ssid.c_str(), MAX_LENGTH_SSID) == 0) {
      return true;
    }
  }
  return false;
}

float matchPercentage(storageStruct &fingerprints, wifiStruct *list) {
  float possibleMatches = 0;
  float matches = 0;
  for (auto fingerprintNum = 0; fingerprintNum < MAX_NETWORKS;
       fingerprintNum++) {
    networkDataStruct &fingerprint = fingerprints.fingerprint[fingerprintNum];
    if (strlen(fingerprint.SSID) > 0 &&
        fingerprint.timesSeen >= MIN_TIMES_CAN_BE_SEEN) {
      possibleMatches++;
      wifiStruct *match = findSSIDInList(list, fingerprint.SSID);
      if (match != nullptr) {
        auto lowerLimit =
            fingerprint.rollingAverageRSSI - fingerprint.rollingVarianceRSSI;
        auto upperLimit =
            fingerprint.rollingAverageRSSI + fingerprint.rollingVarianceRSSI;
        if (match->RSSI >= lowerLimit && match->RSSI <= upperLimit) {
          matches++;
        }
      }
    }
  }
  return matches / possibleMatches;
}

void update(storageStruct &fingerprints, wifiStruct *list) {
  wifiStruct *currentList = list;
  for (auto fingerprintNum = 0; fingerprintNum < MAX_NETWORKS;
       fingerprintNum++) {
    networkDataStruct &fingerprint = fingerprints.fingerprint[fingerprintNum];
    if (strlen(fingerprint.SSID) > 0) {
      wifiStruct *matchingList = findSSIDInList(list, fingerprint.SSID);
      if (matchingList == nullptr) {
        // Update times seen
        if (fingerprint.timesSeen > 0) {
          fingerprint.timesSeen--;
        }
      } else {
        // Update times seen
        if (fingerprint.timesSeen < MAX_TIMES_CAN_BE_SEEN) {
          fingerprint.timesSeen++;
        }
        // update stats
        fingerprint.rollingAverageRSSI -=
            fingerprint.rollingAverageRSSI / SAMPLES;
        fingerprint.rollingAverageRSSI += currentList->RSSI / SAMPLES;
        float difference =
            abs(currentList->RSSI - fingerprint.rollingAverageRSSI);
        if (fingerprint.rollingVarianceRSSI < MIN_ROLLING_VARIANCE) {
          fingerprint.rollingVarianceRSSI = MIN_ROLLING_VARIANCE;
        }
        if (difference > MIN_ROLLING_VARIANCE) {
          fingerprint.rollingVarianceRSSI -=
              fingerprint.rollingVarianceRSSI / SAMPLES;
          fingerprint.rollingVarianceRSSI += difference / SAMPLES;
        }
      }
    }

    if (fingerprint.timesSeen <= MIN_TIMES_CAN_BE_SEEN) {
      // Add strongest network to empty fingerprint slot
      while (currentList != nullptr) {
        if (!isSSIDInFingerprints(fingerprints, currentList->SSID)) {
          strncpy(fingerprint.SSID, currentList->SSID.c_str(), MAX_LENGTH_SSID);
          fingerprint.rollingAverageRSSI = currentList->RSSI;
          fingerprint.rollingVarianceRSSI = MIN_ROLLING_VARIANCE;
          fingerprint.timesSeen =
              (MAX_TIMES_CAN_BE_SEEN - MIN_TIMES_CAN_BE_SEEN) / 3;
          currentList = currentList->next;
          break;
        }
        currentList = currentList->next;
      }
    }
  }
}

}  // namespace Fingerprints