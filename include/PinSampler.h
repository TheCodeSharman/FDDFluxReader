#ifndef PIN_SAMPLER_H
#define PIN_SAMPLER_H

#include <Arduino.h>
#include <RingBuf.h>

#include "MultiTask.h"

class PinSampler {
    private:
        static PinSampler *getInstanceFromHdma(DMA_HandleTypeDef *hdma);
        static TIM_HandleTypeDef *getTimerHandleFromHdma(DMA_HandleTypeDef *hdma);
        static void timerDmaCaptureComplete(DMA_HandleTypeDef *hdma);
        static void timerDmaCaptureHalfComplete(DMA_HandleTypeDef *hdma);

        MultiTask& multitask;
        USBSerial& output;
        const uint8_t pin;
        double ticksToMicros;

        DMA_HandleTypeDef hdma;
        uint32_t channel;

        // Last counter value processed, this is kept so that when the circular buffer
        // is overwritten we know the previous highest sample.
        uint32_t prevSample; 

        // Circular buffer of incoming input capture samples
        uint32_t dmaBuffer[100];

        enum BUFFER_HALF { TOP, BOTTOM };
        void processHalfDmaBuffer(BUFFER_HALF half);

        // A ring buffer to queue the samples as they procesed by the DMA
        // interrupts. 
        RingBuf<uint32_t, 1024> samples;

        // Overflow flag - set to true if the buffer has filled up and
        // samples are being lost.
        volatile bool sampleBufferOverflow;

        HardwareTimer timer;

        MultiTask::CallbackFunction* drainCallback;
        void drainSampleBuffer();

        

    public:
        PinSampler(USBSerial& output, MultiTask& multitask, const uint8_t pin);
        void init();
        void startSampling();
        void stopSampling();
};

#endif