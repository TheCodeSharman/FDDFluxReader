#include "PinSampler.h"


DMA_HandleTypeDef* DMA1_Stream6_hdma;

extern "C" {
  void DMA1_Stream6_IRQHandler(void)
  {
    HAL_DMA_IRQHandler(DMA1_Stream6_hdma);
  }
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
    HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
    __HAL_LINKDMA(timer.getHandle(),hdma[TIM_DMA_ID_CC2],hdma);

    DMA1_Stream6_hdma = &hdma;

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

    drainCallback = multitask.every(0, std::bind(&PinSampler::drainSampleBuffer,this), false);
 }

void PinSampler::drainSampleBuffer() {
    uint32_t sample;
    for( int i = 0; i < 10 && samples.pop(sample); i++) {
        output.printf("%0.3fms\n", sample*ticksToMicros);
    }
}

void PinSampler::processDmaBuffer() {
  // Queue the buffered samples for processing and in the process
  // convert from counter ticks to the number of ticks since the previous
  // capture.
  uint32_t prevSample  = 0;
  for( uint32_t currentSample : dmaBuffer ) {
    uint32_t pulseWidth;

    // We need to detect when the counter rolls over and correct for this 
    // in the caculation.
    if ( currentSample < prevSample ) {
      pulseWidth = 0xFFFF + currentSample - prevSample;
    } else {
      pulseWidth = currentSample - prevSample;
    }
    samples.push(pulseWidth);
    prevSample = currentSample;
  }
}

void PinSampler::timerDmaCaptureComplete(DMA_HandleTypeDef *hdma) {
  TIM_HandleTypeDef *htim = (TIM_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  TIM_CHANNEL_STATE_SET(htim, TIM_CHANNEL_2, HAL_TIM_CHANNEL_STATE_READY);
  TIM_CHANNEL_N_STATE_SET(htim, TIM_CHANNEL_2, HAL_TIM_CHANNEL_STATE_READY);

  // Pointer magic to find the PinSampler instance that this hdam pointer is contained in.
  PinSampler *instance = reinterpret_cast<PinSampler *>((char *)hdma - offsetof(PinSampler, hdma));
  instance->processDmaBuffer();
  
  htim->Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;
}

void PinSampler::startSampling() {
  TIM_HandleTypeDef *halHandle;
  int halChannel;

  halHandle = timer.getHandle();
  halChannel = timer.getChannel(channel); 

  TIM_CHANNEL_STATE_SET(halHandle, halChannel, HAL_TIM_CHANNEL_STATE_BUSY);
  TIM_CHANNEL_N_STATE_SET(halHandle, halChannel, HAL_TIM_CHANNEL_STATE_BUSY);

  TIM_CCxChannelCmd(halHandle->Instance, halChannel, TIM_CCx_ENABLE);

  /* Set the DMA capture callbacks */
  halHandle->hdma[TIM_DMA_ID_CC2]->XferCpltCallback = &PinSampler::timerDmaCaptureComplete;
  halHandle->hdma[TIM_DMA_ID_CC2]->XferHalfCpltCallback = TIM_DMACaptureHalfCplt;

  /* Set the DMA error callback */
  halHandle->hdma[TIM_DMA_ID_CC2]->XferErrorCallback = TIM_DMAError ;

  /* Enable the DMA stream */
  if (HAL_DMA_Start_IT(halHandle->hdma[TIM_DMA_ID_CC2], (uint32_t)&halHandle->Instance->CCR2, (uint32_t) &dmaBuffer, 100) != HAL_OK)
  {
    Error_Handler();
  }
  /* Enable the TIM Capture/Compare 2  DMA request */
  __HAL_TIM_ENABLE_DMA(halHandle, TIM_DMA_CC2);
  __HAL_TIM_ENABLE(halHandle);

  drainCallback->start();
}

void PinSampler::stopSampling() {
  drainCallback->stop();
}