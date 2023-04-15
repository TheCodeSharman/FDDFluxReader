#ifndef PIN_SAMPLER_H
#define PIN_SAMPLER_H

#include <Arduino.h>
#include <RingBuf.h>

#include "MultiTask.h"

class PinSampler {
    private:
        MultiTask& multitask;
        Stream& output;
        const uint8_t pin;

        double ticksToMicros;

        uint32_t channel;
        volatile int32_t lastCapture, currentCapture, rolloverCount;

        //RingBuf<uint32_t, 500> samples;
        volatile uint32_t sampleWriteIndex, sampleReadIndex;

        struct sample_data_t {
            uint32_t sample;
            uint32_t lastCapture;
            uint32_t currentCapture;
            uint32_t rolloverCount;
        };

        sample_data_t samples[500];

        HardwareTimer timer;

        MultiTask::CallbackFunction* drainCallback;

        void captureInterrupt();
        void rolloverInterrupt();

        void drainSampleBuffer();

    public:
        PinSampler(Stream& output, MultiTask& multitask, const uint8_t pin);
        void init();
        void startSampling();
        void stopSampling();
};

#endif