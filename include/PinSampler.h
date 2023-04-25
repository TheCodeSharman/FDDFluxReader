#ifndef PIN_SAMPLER_H
#define PIN_SAMPLER_H

#include <Arduino.h>
#include <RingBuf.h>

#include "MultiTask.h"

class PinSampler {
    public:
        enum State {
            NOT_INITIALISED,
            WAITING_FOR_INDEX,
            SAMPLING,
            STOPPING_SAMPLING,
            ERROR,
            IDLE
        };
        
    private:
        const int NUMBER_OF_SAMPLES_IN_BATCH = 42;

        static PinSampler *getInstanceFromHdma(DMA_HandleTypeDef *hdma);
        static TIM_HandleTypeDef *getTimerHandleFromHdma(DMA_HandleTypeDef *hdma);
        static void timerDmaCaptureComplete(DMA_HandleTypeDef *hdma);
        static void timerDmaCaptureHalfComplete(DMA_HandleTypeDef *hdma);
        
        MultiTask& multitask;
        USBSerial& output;
        const uint32_t readPin, indexPin;

        uint32_t clockFrequency;

        volatile State currentState;
        volatile uint32_t indexStart;
        volatile uint32_t indexStop;
        volatile uint32_t indexPeriod;
        
        inline uint32_t ticksTo25ns(uint32_t ticks) {
            return ((ticks*1000)/clockFrequency)/25;
        }

        DMA_HandleTypeDef hdma;
        uint32_t samplerChannel;
        uint32_t indexChannel;

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

        HardwareTimer samplerTimer;
        HardwareTimer indexTimer;

        MultiTask::CallbackFunction* processSampleBufferCallback;
        void processSampleBuffer();
        void indexHolePassing();
        void sendOutputBuffer(const int count);
        void checkForOverflow();

    public:
        PinSampler(USBSerial& output, MultiTask& multitask, const uint32_t readPin, const uint32_t indexPin);
        void init();
        void startSampling();
        void stopSampling();
        State getState() { return currentState; };
};

#endif