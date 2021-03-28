#ifndef SCAN_H
#define SCAN_H

#include <ESP8266WiFi.h>

#include "utils.h"

static const auto MAX_LENGTH_SSID = 32;
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

struct wifiStruct {
  String SSID;
  float RSSI = 0;
  wifiStruct *next;
};

void loadFingerprints();
void saveFingerprints();
void updateNetworks(wifiStruct *list);
void deleteWifiList(wifiStruct *list);
void printWifiList(wifiStruct *list);
wifiStruct *getOrderedWifiList(int numberOfNetworks);

#endif