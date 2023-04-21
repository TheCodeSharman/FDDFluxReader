#ifndef PIN_SAMPLER_H
#define PIN_SAMPLER_H



#include <Arduino.h>
#include <RingBuf.h>

#include "MultiTask.h"

/*
FIXME:
   If we use the HardwareTimer class we can't trap the DMA interrupt
   because HAL_TIM_IC_CaptureCallback is defined as part of that
   library.

   So instead we implement starting the DMA ourselves in order to
   set the DMA complete handler.

   This is a bit ugly because we're relying on the HAL internals to 
   remain the same.

   So the other option is to use HAL directly ourselves.

   But I haven't got that working yet so hence the switch.
*/
 
#define USE_HARDWARE_TIMER_LIBRARY

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

#ifdef USE_HARDWARE_TIMER_LIBRARY
        HardwareTimer timer;
#else
        TIM_HandleTypeDef htim;
        TIM_ClockConfigTypeDef sClockSourceConfig = {0};
        TIM_MasterConfigTypeDef sMasterConfig = {0};
        TIM_IC_InitTypeDef sConfigIC = {0};

        int getChannel(uint32_t channel);
        void enableTimerClock(TIM_HandleTypeDef *htim);
#endif



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