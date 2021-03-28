#include "scan.h"

#include <EEPROM.h>

#include "utils.h"

storageStruct fingerprints;

void loadFingerprints() {
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
}

void saveFingerprints() {
  EEPROM.put(0, fingerprints);
  EEPROM.commit();
}

void printWifi(wifiStruct *list) {
  Serial.print(list->SSID);
  Serial.print(F(" - "));
  Serial.print(list->RSSI);
  Serial.println(F("db"));
}

void printFingerprint(networkDataStruct &fingerprint) {
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

void printWifiList(wifiStruct *list) {
  while (list != nullptr) {
    printWifi(list);
    list = list->next;
  }
}

wifiStruct *findSSIDInList(wifiStruct *list, char *ssid) {
  while (list != nullptr) {
    // Serial.print(list->SSID);
    // Serial.print(F(","));
    if (strncmp(ssid, list->SSID.c_str(), MAX_LENGTH_SSID) == 0) {
      return list;
    }
    list = list->next;
  }
  return NULL;
}

bool isSSIDInFingerprints(const String &ssid) {
  for (auto fingerprintNum = 0; fingerprintNum < MAX_NETWORKS;
       fingerprintNum++) {
    networkDataStruct &fingerprint = fingerprints.fingerprint[fingerprintNum];
    if (strncmp(fingerprint.SSID, ssid.c_str(), MAX_LENGTH_SSID) == 0) {
      return true;
    }
  }
  return false;
}

void updateNetworks(wifiStruct *list) {
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
        if (!isSSIDInFingerprints(currentList->SSID)) {
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
    printFingerprint(fingerprint);
  }
}

void deleteWifiList(wifiStruct *list) {
  while (list != nullptr) {
    wifiStruct *toDelete = list;
    list = list->next;
    delete toDelete;
  }
}

wifiStruct *getOrderedWifiList(int numberOfNetworks) {
  wifiStruct *root = NULL;
  for (int networkIndex = 0; networkIndex < numberOfNetworks; networkIndex++) {
    auto signalStrength = WiFi.RSSI(networkIndex);
    String ssid = WiFi.SSID(networkIndex);
    wifiStruct *current;
    if (root == nullptr) {
      root = new wifiStruct;
      current = root;
      current->next = NULL;
    } else if (root->RSSI < signalStrength) {
      wifiStruct *temp = root;
      root = new wifiStruct;
      root->next = temp;
      current = root;
    } else {
      current = root;
      while (current->next != nullptr && current->next->RSSI > signalStrength) {
        current = current->next;
      }
      if (current->next == nullptr) {
        current->next = new wifiStruct;
        current = current->next;
        current->next = NULL;
      } else {
        wifiStruct *temp = current->next;
        current->next = new wifiStruct;
        current = current->next;
        current->next = temp;
      }
    }
    current->SSID = ssid;
    current->RSSI = signalStrength;
    // Serial.print(F("Added: "));
    // printWifi(current);
  }
  return root;
}