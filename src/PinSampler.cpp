#include "PinSampler.h"

PinSampler::PinSampler(Stream& output, MultiTask& multitask, const uint8_t pin) 
    : output(output), multitask(multitask), pin(pin) {
}

void PinSampler::init() {
    PinName pinname = digitalPinToPinName(pin);
    TIM_TypeDef *instance = (TIM_TypeDef *)pinmap_peripheral(pinname, PinMap_TIM);
    channel = STM_PIN_CHANNEL(pinmap_function(pinname, PinMap_TIM));

    timer.setup(instance);
    timer.setMode(channel, TIMER_INPUT_CAPTURE_RISING, pinname);
    timer.setPrescaleFactor(1);
    timer.setOverflow(0xFFFF); 
    timer.attachInterrupt(channel, std::bind(&PinSampler::captureInterrupt, this));
    timer.attachInterrupt(std::bind(&PinSampler::rolloverInterrupt, this));

    lastCapture = 0;
    currentCapture = 0;
    ticksToMicros = 1000000.0/timer.getTimerClkFreq();

    drainCallback = multitask.every(1000,std::bind(&PinSampler::drainSampleBuffer, this), false);
}

void PinSampler::drainSampleBuffer() {
    //timer.pause();
    // Print out up to 100 samples before yielding...
   int count = 100;
    // If we're getting this message then we need to increase the buffer size...
    /*if (samples.isFull() ) {
        output.println("Buffer overflow!\n");
    }*/

    // We try to drain the ring buffer before it fills up.
    while( !samples.isEmpty() && count-->0 ) {
        uint32_t sample;
        if ( samples.lockedPop(sample) ) {
            output.printf("%.2fus\n", sample*ticksToMicros);
        }
    }
}

void PinSampler::captureInterrupt() {
    currentCapture = timer.getCaptureCompare(channel);
    samples.push(0xFFFF*rolloverCount + currentCapture - lastCapture);
    lastCapture = currentCapture;
    rolloverCount=0;
}

void PinSampler::rolloverInterrupt() {
    rolloverCount++;
}

void PinSampler::startSampling() {
    samples.clear();
    timer.resume();
    drainCallback->start();
}

void PinSampler::stopSampling() {
    timer.pause();
    // completely drain the sample buffer
    while( !samples.isEmpty() ){
        drainSampleBuffer();
    }
    drainCallback->stop();
}