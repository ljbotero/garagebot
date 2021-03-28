#ifndef SCAN_H
#define SCAN_H

#include <ESP8266WiFi.h>

#include "utils.h"

struct wifiStruct {
  String SSID;
  float RSSI = 0;
  wifiStruct *next;
};

void deleteWifiList(wifiStruct *list);
void printWifiList(wifiStruct *list);
wifiStruct *findSSIDInList(wifiStruct *list, char *ssid);
wifiStruct *getOrderedWifiList(int numberOfNetworks);

#endif