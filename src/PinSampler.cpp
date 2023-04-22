#include "PinSampler.h"


DMA_HandleTypeDef* DMA1_Stream6_hdma;

extern "C" {
  void DMA1_Stream6_IRQHandler(void)
  {
    HAL_DMA_IRQHandler(DMA1_Stream6_hdma);
  }
#ifndef USE_HARDWARE_TIMER_LIBRARY
  void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
  {
    PinSampler::HALInputCapture(htim);
  }

  void PinSampler::HALInputCapture(TIM_HandleTypeDef *htim) {
    // Pointer magic to find the PinSampler instance that this htim pointer is contained in.
    PinSampler *instance = reinterpret_cast<PinSampler *>((char *)htim - offsetof(PinSampler, htim));
    instance->DMABufferFull();
  }
#endif
}

PinSampler::PinSampler(Stream& output, MultiTask& multitask, const uint8_t pin) 
    : output(output), multitask(multitask), pin(pin) {
}

#ifndef USE_HARDWARE_TIMER_LIBRARY
int PinSampler::getChannel(uint32_t channel)
{
  uint32_t return_value;

  switch (channel) {
    case 1:
      return_value = TIM_CHANNEL_1;
      break;
    case 2:
      return_value = TIM_CHANNEL_2;
      break;
    case 3:
      return_value = TIM_CHANNEL_3;
      break;
    case 4:
      return_value = TIM_CHANNEL_4;
      break;
    default:
      return_value = -1;
  }
  return return_value;
}

void PinSampler::enableTimerClock(TIM_HandleTypeDef *htim)
{
  // Enable TIM clock
#if defined(TIM1_BASE)
  if (htim->Instance == TIM1) {
    __HAL_RCC_TIM1_CLK_ENABLE();
  }
#endif
#if defined(TIM2_BASE)
  if (htim->Instance == TIM2) {
    __HAL_RCC_TIM2_CLK_ENABLE();
  }
#endif
#if defined(TIM3_BASE)
  if (htim->Instance == TIM3) {
    __HAL_RCC_TIM3_CLK_ENABLE();
  }
#endif
#if defined(TIM4_BASE)
  if (htim->Instance == TIM4) {
    __HAL_RCC_TIM4_CLK_ENABLE();
  }
#endif
#if defined(TIM5_BASE)
  if (htim->Instance == TIM5) {
    __HAL_RCC_TIM5_CLK_ENABLE();
  }
#endif
#if defined(TIM6_BASE)
  if (htim->Instance == TIM6) {
    __HAL_RCC_TIM6_CLK_ENABLE();
  }
#endif
#if defined(TIM7_BASE)
  if (htim->Instance == TIM7) {
    __HAL_RCC_TIM7_CLK_ENABLE();
  }
#endif
#if defined(TIM8_BASE)
  if (htim->Instance == TIM8) {
    __HAL_RCC_TIM8_CLK_ENABLE();
  }
#endif
#if defined(TIM9_BASE)
  if (htim->Instance == TIM9) {
    __HAL_RCC_TIM9_CLK_ENABLE();
  }
#endif
#if defined(TIM10_BASE)
  if (htim->Instance == TIM10) {
    __HAL_RCC_TIM10_CLK_ENABLE();
  }
#endif
#if defined(TIM11_BASE)
  if (htim->Instance == TIM11) {
    __HAL_RCC_TIM11_CLK_ENABLE();
  }
#endif
#if defined(TIM12_BASE)
  if (htim->Instance == TIM12) {
    __HAL_RCC_TIM12_CLK_ENABLE();
  }
#endif
#if defined(TIM13_BASE)
  if (htim->Instance == TIM13) {
    __HAL_RCC_TIM13_CLK_ENABLE();
  }
#endif
#if defined(TIM14_BASE)
  if (htim->Instance == TIM14) {
    __HAL_RCC_TIM14_CLK_ENABLE();
  }
#endif
#if defined(TIM15_BASE)
  if (htim->Instance == TIM15) {
    __HAL_RCC_TIM15_CLK_ENABLE();
  }
#endif
#if defined(TIM16_BASE)
  if (htim->Instance == TIM16) {
    __HAL_RCC_TIM16_CLK_ENABLE();
  }
#endif
#if defined(TIM17_BASE)
  if (htim->Instance == TIM17) {
    __HAL_RCC_TIM17_CLK_ENABLE();
  }
#endif
#if defined(TIM18_BASE)
  if (htim->Instance == TIM18) {
    __HAL_RCC_TIM18_CLK_ENABLE();
  }
#endif
#if defined(TIM19_BASE)
  if (htim->Instance == TIM19) {
    __HAL_RCC_TIM19_CLK_ENABLE();
  }
#endif
#if defined(TIM20_BASE)
  if (htim->Instance == TIM20) {
    __HAL_RCC_TIM20_CLK_ENABLE();
  }
#endif
#if defined(TIM21_BASE)
  if (htim->Instance == TIM21) {
    __HAL_RCC_TIM21_CLK_ENABLE();
  }
#endif
#if defined(TIM22_BASE)
  if (htim->Instance == TIM22) {
    __HAL_RCC_TIM22_CLK_ENABLE();
  }
#endif
}
#endif

void PinSampler::init() {
    PinName pinname = digitalPinToPinName(pin);
    TIM_TypeDef *instance = (TIM_TypeDef *)pinmap_peripheral(pinname, PinMap_TIM);
    channel = STM_PIN_CHANNEL(pinmap_function(pinname, PinMap_TIM));

#ifdef USE_HARDWARE_TIMER_LIBRARY
    timer.setup(instance);
    timer.setMode(channel, TIMER_INPUT_CAPTURE_FALLING, pinname);
    timer.setPrescaleFactor(1);
    timer.setOverflow(0xFFFF); 
#else
    htim.Instance = instance;
    htim.hdma[0] = NULL;
    htim.hdma[1] = NULL;
    htim.hdma[2] = NULL;
    htim.hdma[3] = NULL;
    htim.hdma[4] = NULL;
    htim.hdma[5] = NULL;
    htim.hdma[6] = NULL;
    htim.Lock = HAL_UNLOCKED;
    htim.State = HAL_TIM_STATE_RESET;
    htim.Init.Prescaler = 0;
    htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim.Init.Period = 0xFFFF;
    htim.Init.RepetitionCounter = 0;
    htim.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    enableTimerClock(&htim);

    if (HAL_TIM_Base_Init(&htim) != HAL_OK)
    {
      Error_Handler();
    }

    if (HAL_TIM_IC_Init(&htim) != HAL_OK)
    {
      Error_Handler();
    }

    sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
    sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
    sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
    sConfigIC.ICFilter = 0;
    if (HAL_TIM_IC_ConfigChannel(&htim, &sConfigIC, getChannel(channel)) != HAL_OK)
    {
      Error_Handler();
    }
#endif


    ticksToMicros = 1000000.0/HAL_RCC_GetHCLKFreq();

    /* TIM2 DMA Init */
    /* TIM2_CH2 Init */
    __HAL_RCC_DMA1_CLK_ENABLE();
    HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);

#ifdef USE_HARDWARE_TIMER_LIBRARY
    __HAL_LINKDMA(timer.getHandle(),hdma[TIM_DMA_ID_CC2],hdma);
#else
    __HAL_LINKDMA(&htim,hdma[TIM_DMA_ID_CC2],hdma);
#endif

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

    drainCallback = multitask.every(5000, std::bind(&PinSampler::drainSampleBuffer,this), false);
 }

void PinSampler::drainSampleBuffer() {
    uint32_t sample;
    for( int i = 0; i < 10 && samples.pop(sample); i++) {
        output.printf("%0.3fms\n", sample*ticksToMicros);
    }
}

void PinSampler::DMABufferFull() {
  // Queue the 100 dma buffered samples for processing and in the process
  // convert from counter ticks to the number of ticks since the previous
  // capture.
  uint32_t prevSample  = 0;
  for( int i = 0; i<100; i++ ) {
    uint32_t currentSample = dmaBuffer[i];
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

#ifdef USE_HARDWARE_TIMER_LIBRARY
void PinSampler::DMACaptureComplete(DMA_HandleTypeDef *hdma) {
  TIM_HandleTypeDef *htim = (TIM_HandleTypeDef *)((DMA_HandleTypeDef *)hdma)->Parent;

  TIM_CHANNEL_STATE_SET(htim, TIM_CHANNEL_2, HAL_TIM_CHANNEL_STATE_READY);
  TIM_CHANNEL_N_STATE_SET(htim, TIM_CHANNEL_2, HAL_TIM_CHANNEL_STATE_READY);

  // Pointer magic to find the PinSampler instance that this hdam pointer is contained in.
  PinSampler *instance = reinterpret_cast<PinSampler *>((char *)hdma - offsetof(PinSampler, hdma));
  instance->DMABufferFull();
  

  htim->Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;
}
#endif


void PinSampler::startSampling() {
  TIM_HandleTypeDef *halHandle;
  int halChannel;

#ifdef USE_HARDWARE_TIMER_LIBRARY
  halHandle = timer.getHandle();
  halChannel = timer.getChannel(channel); 
#else
  halHandle = &htim;
  halChannel = getChannel(channel); 
#endif

#ifndef USE_HARDWARE_TIMER_LIBRARY
    if (HAL_TIM_IC_Start_DMA(halHandle, halChannel, dmaBuffer, 100) != HAL_OK)
    {
      Error_Handler();
    } 
#else
    TIM_CHANNEL_STATE_SET(halHandle, halChannel, HAL_TIM_CHANNEL_STATE_BUSY);
    TIM_CHANNEL_N_STATE_SET(halHandle, halChannel, HAL_TIM_CHANNEL_STATE_BUSY);

    TIM_CCxChannelCmd(halHandle->Instance, halChannel, TIM_CCx_ENABLE);

    /* Set the DMA capture callbacks */
    halHandle->hdma[TIM_DMA_ID_CC2]->XferCpltCallback = &PinSampler::DMACaptureComplete;
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

#endif

#if 0
    delay(100);
    uint32_t prevSample = 0;
    for( int i = 0; i<10; i++ ) {
      uint32_t currentSample = dmaBuffer[i];
      uint32_t pulseWidth;

      // We need to detect when the counter rolls over and correct for this 
      // in the caculation.
      if ( currentSample < prevSample ) {
        pulseWidth = 0xFFFF + currentSample - prevSample;
      } else {
        pulseWidth = currentSample - prevSample;
      }
      output.printf("%0.3fms\n", pulseWidth*ticksToMicros);
      prevSample = currentSample;
    }
#else
    drainCallback->start();
#endif
}

void PinSampler::stopSampling() {
  drainCallback->stop();
}