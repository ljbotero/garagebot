#include "scan.h"

void printWifi(wifiStruct *list) {
  Serial.print(list->SSID);
  Serial.print(F(" - "));
  Serial.print(list->RSSI);
  Serial.println(F("db"));
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