#define CORE_DEBUG_LEVEL 5
//#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#include <Arduino.h>
#include <SPI.h>
#include "Gateway.h"

Gateway gateway;

void setup() {
  Serial.begin(115200);
  log_d("*start*");

  gateway.setup();
  
  log_d("*end*");
}

void loop() {
  gateway.loop();
}