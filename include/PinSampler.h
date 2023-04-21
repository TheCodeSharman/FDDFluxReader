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

        DMA_HandleTypeDef hdma;
        uint32_t channel;
        uint32_t dmaBuffer[100];

        RingBuf<uint32_t, 100> samples;

        HardwareTimer timer;
        MultiTask::CallbackFunction* drainCallback;

        static void DMACaptureComplete(DMA_HandleTypeDef *hdma);

        void drainSampleBuffer();

    public:
        PinSampler(Stream& output, MultiTask& multitask, const uint8_t pin);
        void init();
        void startSampling();
        void stopSampling();
};

#endif