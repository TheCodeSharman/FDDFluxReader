#include <Arduino.h>

#include "MultiTask.h"
#include "CommandProcessor.h"
#include "PinSampler.h"

const uint32_t LED_PIN = PC13;
const uint32_t READ_PIN = PA1;
const uint32_t INDEX_PIN = PB0;
const uint32_t DIR_PIN = PB9;
const uint32_t STEP_PIN = PB8;


MultiTask tasks;
PinSampler readSampler(Serial,tasks,READ_PIN,INDEX_PIN,DIR_PIN,STEP_PIN);
CommandProcessor command(Serial,tasks,readSampler);

void blinkLed() {
  static bool isLedOn = false;
  digitalWrite(LED_PIN, (isLedOn?HIGH:LOW));
  isLedOn = !isLedOn;
}

HardwareTimer testSignal;

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(READ_PIN, INPUT);   
  digitalWrite(LED_PIN, LOW);
  tasks.every(500000,blinkLed);
  command.init();
  readSampler.init();
}

void loop() {
  tasks.process();
}