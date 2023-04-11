#include <Arduino.h>

#include "MultiTask.h"
#include "CommandProcessor.h"
#include "ReadSampler.h"

const uint8_t LED_PIN = PC13;
const uint8_t READ_PIN = PA10;

MultiTask tasks;
ReadSampler readSampler(Serial,tasks,READ_PIN);
CommandProcessor command(Serial,tasks,readSampler);

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
  readSampler.init();
}

void loop() {
  tasks.process();
}