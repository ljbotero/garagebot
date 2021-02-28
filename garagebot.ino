#include <ESP8266WiFi.h>

#include "globals.h"
#include "utils.h"

unsigned long last_time_seen = 0;
auto is_scanning = false;
uint16_t normalized_signal_strenght;

void setup() {
  Serial.println(F("Setup"));
  Serial.begin(115200, SERIAL_8N1, SERIAL_TX_ONLY);
  while (!Serial) {
    delay(1);
  }
  pinMode(ON_BOARD_LED, OUTPUT);  // Initialize the LED_BUILTIN pin as an output
  pinMode(BUTTON_PIN,
          INPUT_PULLUP);  // Initialize button pin with built-in pullup.
  analogWriteRange(255);
  analogWrite(RGB_R_PIN, 0);
  analogWrite(RGB_G_PIN, 0);
  analogWrite(RGB_B_PIN, 0);
  digitalWrite(ON_BOARD_LED, HIGH);  // Off
  last_time_seen = millis();
}

void openGarage(uint32_t durationMillis = 2000) {
  Serial.println(F("OPENING GARAGE"));
  analogWrite(RGB_G_PIN, 0);
  analogWrite(RGB_R_PIN, 255);
  delay(durationMillis);
  analogWrite(RGB_R_PIN, 0);
}

void scan(int numberOfNetworks) {
  Serial.print(F("Networks: "));
  bool found = false;
  for (int networkIndex = 0; networkIndex < numberOfNetworks; networkIndex++) {
    String currentSSID = WiFi.SSID(networkIndex);
    Serial.print(currentSSID);
    Serial.print(F(" ,"));
    if (currentSSID.equalsIgnoreCase(NETWORK_NAME)) {
      auto signalStrength = WiFi.RSSI(networkIndex);
      updateRSSIStats(signalStrength);
      if (isRSSIOutsideNormal(signalStrength)) {
        if (last_time_seen + HOW_LONG_BEFORE_OPENING_GARAGE < millis()) {
          openGarage();
        } else {
          analogWrite(RGB_B_PIN, 255);
          delay(200);
          analogWrite(RGB_B_PIN, 0);
        }
      } else {
        last_time_seen = millis();
      }
      auto mappedValue = map(signalStrength, -85, -35, 1, 255);
      normalized_signal_strenght = constrain(mappedValue, 1, 255);
      Serial.println(F(""));
      Serial.print(F("Found network: "));
      Serial.print(signalStrength);
      Serial.print(F("db ("));
      Serial.print(normalized_signal_strenght);
      Serial.println(F(")"));
      found = true;
      break;
    }
  }
  if (!found) {
    normalized_signal_strenght = 0;
  }
  is_scanning = false;
}

void blink() {
  if (normalized_signal_strenght == 0) {
    if (last_time_seen + HOW_LONG_BEFORE_OPENING_GARAGE < millis()) {
      blinkLight(RGB_B_PIN, 200);  // Blink blue when ready to trigger
    } else {
      blinkLight(RGB_R_PIN, 200);  // Blink red when un-engaged
    }
  } else {
    auto blinkDelay = map(normalized_signal_strenght, 1, 255, 2000, 100);
    blinkLight(RGB_G_PIN, blinkDelay);
  }
}

void loop() {
  auto btn_Status = digitalRead(BUTTON_PIN);
  if (btn_Status == LOW) {
    openGarage(500);
  }

  if (!is_scanning) {
    WiFi.scanNetworks(true, false);
    is_scanning = true;
  }
  int numberOfNetworks = WiFi.scanComplete();
  if (numberOfNetworks >= 0) {
    scan(numberOfNetworks);
    WiFi.scanDelete();
  }

  blink();
}
