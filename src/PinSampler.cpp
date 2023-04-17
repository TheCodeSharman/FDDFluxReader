#include "PinSampler.h"


DMA_HandleTypeDef hdma;

void DMA1_Stream6_IRQHandler(void)
{
  HAL_DMA_IRQHandler(&hdma);
}

PinSampler::PinSampler(Stream& output, MultiTask& multitask, const uint8_t pin) 
    : output(output), multitask(multitask), pin(pin) {
}

void PinSampler::init() {
    PinName pinname = digitalPinToPinName(pin);
    TIM_TypeDef *instance = (TIM_TypeDef *)pinmap_peripheral(pinname, PinMap_TIM);
    channel = STM_PIN_CHANNEL(pinmap_function(pinname, PinMap_TIM));

    timer.setup(instance);
    timer.setMode(channel, TIMER_INPUT_CAPTURE_FALLING, pinname);
    timer.setPrescaleFactor(1);
    timer.setOverflow(0xFFFF); 

    ticksToMicros = 1000000.0/timer.getTimerClkFreq();

    /* TIM2 DMA Init */
    /* TIM2_CH2 Init */
    __HAL_RCC_DMA1_CLK_ENABLE();
    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
    __HAL_LINKDMA(timer.getHandle(),hdma[TIM_DMA_ID_CC2],hdma);

    hdma.Instance = DMA1_Stream6;
    hdma.Init.Channel = DMA_CHANNEL_3;
    hdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma.Init.MemInc = DMA_MINC_ENABLE;
    hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    hdma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
    hdma.Init.Mode = DMA_NORMAL;
    hdma.Init.Priority = DMA_PRIORITY_LOW;
    hdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
    hdma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
    hdma.Init.MemBurst = DMA_MBURST_SINGLE;
    hdma.Init.PeriphBurst = DMA_PBURST_SINGLE;
    if (HAL_DMA_Init(&hdma) != HAL_OK)
    {
      Error_Handler();
    }


 }

void PinSampler::drainSampleBuffer() {
    uint32_t prevSample  = 0;
    for( int i = 0; i < 50; i++) {
        uint32_t currentSample = samples[i];
        output.printf("%0.3fms\n", (samples[i] - prevSample)*ticksToMicros);
        prevSample = currentSample;
    }
}


void PinSampler::startSampling() {
    auto halHandle = timer.getHandle();
    auto halChannel = timer.getChannel(channel);
    if (HAL_TIM_IC_Start_DMA(halHandle, halChannel, samples, 500) != HAL_OK)
    {
      Error_Handler();
    }
    delay(1000);
    drainSampleBuffer();
    if (HAL_TIM_IC_Stop_DMA(halHandle, halChannel) != HAL_OK)
    {
      Error_Handler();
    }
}

void PinSampler::stopSampling() {
}