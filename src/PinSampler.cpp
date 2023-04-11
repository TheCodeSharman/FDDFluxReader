#include "PinSampler.h"

PinSampler::PinSampler(Stream& output, MultiTask& multitask, const uint8_t pin) 
    : output(output), multitask(multitask), pin(pin) {
}

void PinSampler::init() {
    PinName pinname = digitalPinToPinName(pin);
    TIM_TypeDef *instance = (TIM_TypeDef *)pinmap_peripheral(pinname, PinMap_TIM);
    channel = STM_PIN_CHANNEL(pinmap_function(pinname, PinMap_TIM));

    timer.setup(instance);
    timer.setMode(channel, TIMER_INPUT_CAPTURE_RISING, pin);
    timer.setPrescaleFactor(1);
    timer.setOverflow(0x10000); 
    timer.attachInterrupt(channel, std::bind(&PinSampler::captureInterrupt, *this));
    timer.attachInterrupt(std::bind(&PinSampler::rolloverInterrupt, *this));

    lastCapture = 0;
    currentCapture = 0;

    drainCallback = multitask.every(2000,std::bind(&PinSampler::drainSampleBuffer, *this));
    drainCallback->stop();
}

void PinSampler::drainSampleBuffer() {
    // Print out up to 100 samples before yielding...
    int count = 100;

    // If we're getting this message then we need to increase the buffer size...
    if (samples.isFull() ) {
        output.println("Buffer overflow!\n");
    }

    // We try to drain the ring buffer before it fills up.
    while( !samples.isEmpty() && count-- > 0 ) {
        uint32_t sample;
        if ( samples.pop(sample) ) {
            output.printf("%i\n", sample);
        }
    }
}

void PinSampler::captureInterrupt() {
    currentCapture = timer.getCaptureCompare(channel);
    samples.push(currentCapture - lastCapture);
    lastCapture = currentCapture;
}

void PinSampler::rolloverInterrupt() {
    samples.push((uint32_t)0);
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