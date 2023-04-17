#include <Arduino.h>

#include "MultiTask.h"
#include "CommandProcessor.h"
#include "PinSampler.h"

const uint32_t LED_PIN = PC13;
const uint32_t READ_PIN = PA1;
const uint32_t TEST_SIGNAL_PIN = PA10;

MultiTask tasks;
PinSampler readSampler(Serial,tasks,READ_PIN);
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

  TIM_TypeDef *instance = (TIM_TypeDef *)pinmap_peripheral(digitalPinToPinName(TEST_SIGNAL_PIN), PinMap_TIM);
  uint32_t channel = STM_PIN_CHANNEL(pinmap_function(digitalPinToPinName(TEST_SIGNAL_PIN), PinMap_TIM));
  testSignal.setup(instance);
  testSignal.setPWM(channel, TEST_SIGNAL_PIN, 333333, 50);

  readSampler.init();
}

void loop() {
  tasks.process();
}