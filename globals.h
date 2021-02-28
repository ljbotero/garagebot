#ifndef GLOBALS_H
#define GLOBALS_H

#include <Arduino.h>

// Pinout:
// https://protosupplies.com/product/esp8266-witty-cloud-esp-12f-wifi-module/

const auto NETWORK_NAME = "BOTEROS-NET";
const auto HOW_LONG_BEFORE_OPENING_GARAGE = 10 * 60 * 1000;  // 10 MINUTES

// https://protosupplies.com/product/esp8266-witty-cloud-esp-12f-wifi-module/
const auto BUTTON_PIN = 4;    // Define pin the button is connected to
const auto ON_BOARD_LED = 2;  // Define pin the on-board LED is connected to
const auto RGB_R_PIN = 15;    // RGB Green LED
const auto RGB_G_PIN = 12;    // RGB Green LED
const auto RGB_B_PIN = 13;    // RGB Green LED
const auto LDR_PIN = PIN_A0;  // Define the analog pin the LDR is connected to

#endif