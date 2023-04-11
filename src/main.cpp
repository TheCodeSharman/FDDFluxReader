#include <Arduino.h>

#include "MultiTask.h"
#include "FluxReaderCommandProcessor.h"


const uint8_t LED_PIN = PC13;

MultiTask tasks;
FluxReaderCommandProcessor command(Serial,tasks);

void blinkLed() {
  static bool isLedOn = false;
  digitalWrite(LED_PIN, (isLedOn?HIGH:LOW));
  isLedOn = !isLedOn;
}

void setup() {
  pinMode(LED_PIN, OUTPUT);  
  digitalWrite(LED_PIN, LOW);
  tasks.every(500000,blinkLed);
  command.init();
}



void loop() {
  tasks.process();
}