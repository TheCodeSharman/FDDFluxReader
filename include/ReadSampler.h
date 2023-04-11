#ifndef READ_SAMPLER_H
#define READ_SAMPLER_H

#include <Arduino.h>
#include <RingBuf.h>

#include "MultiTask.h"

class ReadSampler {
    private:
        MultiTask& multitask;
        Stream& output;
        const uint8_t pin;

        uint32_t channel;
        volatile uint32_t lastCapture, currentCapture;

        RingBuf<uint32_t, 1024> samples;
        HardwareTimer timer;

        MultiTask::CallbackFunction* drainCallback;

        void captureInterrupt();
        void rolloverInterrupt();

        void drainSampleBuffer();

    public:
        ReadSampler(Stream& output, MultiTask& multitask, const uint8_t pin);
        void init();
        void startSampling();
        void stopSampling();
};

#endif