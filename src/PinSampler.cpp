#include "PinSampler.h"


DMA_HandleTypeDef hdma_tim2_ch2_ch4;

void DMA1_Stream6_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma_tim2_ch2_ch4);
}

PinSampler::PinSampler(Stream& output, MultiTask& multitask, const uint8_t pin) 
    : output(output), multitask(multitask), pin(pin) {
}

void PinSampler::init() {

    __HAL_RCC_DMA1_CLK_ENABLE();

    PinName pinname = digitalPinToPinName(pin);
    TIM_TypeDef *instance = (TIM_TypeDef *)pinmap_peripheral(pinname, PinMap_TIM);
    channel = STM_PIN_CHANNEL(pinmap_function(pinname, PinMap_TIM));

    timer.setup(instance);
    timer.setMode(channel, TIMER_INPUT_CAPTURE_RISING, pinname);
    timer.setPrescaleFactor(1);
    timer.setOverflow(0xFFFF); 

    ticksToMicros = 1000000.0/timer.getTimerClkFreq();

    /* TIM2 DMA Init */
    /* TIM2_CH2_CH4 Init */
    hdma_tim2_ch2_ch4.Instance = DMA1_Stream6;
    hdma_tim2_ch2_ch4.Init.Channel = DMA_CHANNEL_3;
    hdma_tim2_ch2_ch4.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_tim2_ch2_ch4.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_tim2_ch2_ch4.Init.MemInc = DMA_MINC_ENABLE;
    hdma_tim2_ch2_ch4.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma_tim2_ch2_ch4.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma_tim2_ch2_ch4.Init.Mode = DMA_NORMAL;
    hdma_tim2_ch2_ch4.Init.Priority = DMA_PRIORITY_LOW;
    hdma_tim2_ch2_ch4.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    hdma_tim2_ch2_ch4.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma_tim2_ch2_ch4.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma_tim2_ch2_ch4.Init.PeriphBurst = DMA_PBURST_SINGLE;
    if (HAL_DMA_Init(&hdma_tim2_ch2_ch4) != HAL_OK)
    {
      Error_Handler();
    }

    __HAL_LINKDMA(timer.getHandle(),hdma[TIM_DMA_ID_CC2],hdma_tim2_ch2_ch4);
   // __HAL_LINKDMA(timer.getHandle(),hdma[TIM_DMA_ID_CC4],hdma_tim2_ch2_ch4);

    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);


 }

void PinSampler::drainSampleBuffer() {
    uint32_t prevSample  = 0;
    for( int i = 0; i < 50; i++) {
        uint32_t currentSample = samples[i];
        output.printf("%0.6fms\n", (samples[i] - prevSample)*ticksToMicros);
        prevSample = currentSample;
    }
}


void PinSampler::startSampling() {
    auto halHandle = timer.getHandle();
    auto halChannel = timer.getChannel(channel);
    if ( auto status = HAL_TIM_IC_Start_DMA(halHandle, halChannel, samples, 500) != HAL_OK)
    {
      Error_Handler();
    }

    //HAL_DMA_PollForTransfer(&hdma_tim2_ch2_ch4,HAL_DMA_FULL_TRANSFER,100);
    delay(1000);
    drainSampleBuffer();
}

void PinSampler::stopSampling() {
    timer.pause();
    drainCallback->stop();
}