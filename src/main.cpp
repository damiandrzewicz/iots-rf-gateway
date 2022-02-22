#define CORE_DEBUG_LEVEL 5
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include <Arduino.h>
#include <ArduinoLog.h>
#include <SPI.h>
#include "Gateway.hpp"
#include <EEPROM.h>

Gateway gateway;

void setup() {
  Serial.begin(115200);

  // Initialize logger
  Log.begin(LOG_LEVEL_VERBOSE, &Serial);

  gateway.setup();
}

void loop() {
  gateway.loop();
}