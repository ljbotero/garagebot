#include "fingerprints.h"

#include <EEPROM.h>

#include "scan.h"

namespace Fingerprints {

storageStruct &load() {
  storageStruct fingerprints;
  EEPROM.begin(sizeof(fingerprints));
  EEPROM.get(0, fingerprints);
  if (fingerprints.state != STATE_INITIALIZED) {
    storageStruct newData;
    newData.state = STATE_INITIALIZED;
    EEPROM.put(0, newData);
    EEPROM.commit();
    fingerprints = newData;
    Serial.println(F("EEPROM Initialized"));
  }
  return fingerprints;
}

void save(storageStruct &fingerprints) {
  EEPROM.put(0, fingerprints);
  EEPROM.commit();
}

void print(networkDataStruct &fingerprint) {
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

void update(storageStruct &fingerprints, wifiStruct *list) {
  wifiStruct *currentList = list;
  for (auto fingerprintNum = 0; fingerprintNum < MAX_NETWORKS;
       fingerprintNum++) {
    networkDataStruct &fingerprint = fingerprints.fingerprint[fingerprintNum];
    if (strlen(fingerprint.SSID) > 0) {
      wifiStruct *matchingList = findSSIDInList(list, fingerprint.SSID);
      if (matchingList == nullptr) {
        // Update times seen
        if (fingerprint.timesSeen > MIN_TIMES_CAN_BE_SEEN) {
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
    print(fingerprint);
  }
}

}