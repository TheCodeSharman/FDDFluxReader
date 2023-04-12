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

        uint32_t channel;
        volatile uint32_t lastCapture, currentCapture, interruptCalled;

        RingBuf<uint32_t, 1024> samples;
        HardwareTimer timer;

        MultiTask::CallbackFunction* drainCallback;

        void captureInterrupt();
        void rolloverInterrupt();

        void drainSampleBuffer();

        void dumpTimer() {
            output.printf("timer stats:\n");
            output.printf("count: %i\n", timer.getCount());
            output.printf("captureCount: %i\n", timer.getCaptureCompare(channel));
            output.printf("interruptCalled: %i\n", interruptCalled);
            output.printf("isRunning: %i\n", timer.isRunning());
        }

    public:
        PinSampler(Stream& output, MultiTask& multitask, const uint8_t pin);
        void init();
        void startSampling();
        void stopSampling();
};

#endif