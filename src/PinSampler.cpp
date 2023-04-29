#include "PinSampler.h"
#include <base64.hpp>
#include "utils/buffer.h"

DMA_HandleTypeDef* DMA1_Stream6_hdma;

extern "C" {
  void DMA1_Stream6_IRQHandler(void)
  {
    HAL_DMA_IRQHandler(DMA1_Stream6_hdma);
  }
}

PinSampler::PinSampler(USBSerial& output, MultiTask& multitask, const uint32_t readPin, const uint32_t indexPin) 
    : output(output), multitask(multitask), readPin(readPin), indexPin(indexPin),
      currentState(NOT_INITIALISED) {
}

void PinSampler::init() {
  // Don't initialise if we already have been
  if ( currentState != NOT_INITIALISED )
    return;

  // Configure index hole interrupt
  pinMode(indexPin,INPUT);
  attachInterrupt(digitalPinToInterrupt(indexPin), std::bind(&PinSampler::indexHolePassing, this), FALLING);
  // PinName indexPinname = digitalPinToPinName(indexPin);
  // TIM_TypeDef *indexInstance = (TIM_TypeDef *)pinmap_peripheral(indexPinname, PinMap_TIM);
  // uint32_t indexChannel = STM_PIN_CHANNEL(pinmap_function(indexPinname, PinMap_TIM));

  // indexTimer.setup(indexInstance);
  // indexTimer.setMode(indexChannel, TIMER_INPUT_CAPTURE_FALLING, indexPinname);
  // indexTimer.setPrescaleFactor(8);
  // indexTimer.setOverflow(0xFFFF); 
  // indexTimer.attachInterrupt(indexChannel,std::bind(&PinSampler::indexHolePassing, this));
  // indexTimer.resume();

  // Configure read pin
  PinName readPinname = digitalPinToPinName(readPin);
  TIM_TypeDef *readTimerInstance = (TIM_TypeDef *)pinmap_peripheral(readPinname, PinMap_TIM);
  samplerChannel = STM_PIN_CHANNEL(pinmap_function(readPinname, PinMap_TIM));

  samplerTimer.setup(readTimerInstance);
  samplerTimer.setMode(samplerChannel, TIMER_INPUT_CAPTURE_FALLING, readPinname);
  samplerTimer.setPrescaleFactor(1);
  samplerTimer.setOverflow(0xFFFF); 

  clockFrequency = samplerTimer.getTimerClkFreq()/1000000;

  /* TIM2 DMA Init */
  /* TIM2_CH2 Init */
  __HAL_RCC_DMA1_CLK_ENABLE();
  HAL_NVIC_SetPriority(DMA1_Stream6_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream6_IRQn);
  __HAL_LINKDMA(samplerTimer.getHandle(),hdma[TIM_DMA_ID_CC2],hdma);

  DMA1_Stream6_hdma = &hdma;

  hdma.Instance = DMA1_Stream6;
  hdma.Init.Channel = DMA_CHANNEL_3;
  hdma.Init.Direction = DMA_PERIPH_TO_MEMORY;
  hdma.Init.PeriphInc = DMA_PINC_DISABLE;
  hdma.Init.MemInc = DMA_MINC_ENABLE;
  hdma.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
  hdma.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;
  hdma.Init.Mode = DMA_CIRCULAR;
  hdma.Init.Priority = DMA_PRIORITY_LOW;
  hdma.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
  hdma.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  hdma.Init.MemBurst = DMA_MBURST_SINGLE;
  hdma.Init.PeriphBurst = DMA_PBURST_SINGLE;
  if (HAL_DMA_Init(&hdma) != HAL_OK)
  {
    Error_Handler();
  }

  TIM_HandleTypeDef *halHandle = samplerTimer.getHandle();
  int halChannel = samplerTimer.getChannel(samplerChannel); 

  TIM_CHANNEL_STATE_SET(halHandle, halChannel, HAL_TIM_CHANNEL_STATE_BUSY);
  TIM_CHANNEL_N_STATE_SET(halHandle, halChannel, HAL_TIM_CHANNEL_STATE_BUSY);

  TIM_CCxChannelCmd(halHandle->Instance, halChannel, TIM_CCx_ENABLE);

  /* Set the DMA capture callbacks */
  halHandle->hdma[TIM_DMA_ID_CC2]->XferCpltCallback = &PinSampler::timerDmaCaptureComplete;
  halHandle->hdma[TIM_DMA_ID_CC2]->XferHalfCpltCallback = &PinSampler::timerDmaCaptureHalfComplete;;

  /* Set the DMA error callback */
  halHandle->hdma[TIM_DMA_ID_CC2]->XferErrorCallback = TIM_DMAError;

  processSampleBufferCallback = multitask.every(0, std::bind(&PinSampler::processSampleBuffer,this), false);

  currentState = IDLE;
 }

void PinSampler::sendOutputBuffer(const int count) {
  uint32_t sampleBuffer[count];
  uint8_t outBuffer[encode_base64_length(count*4)+1];
  uint32_t sample;
  
  int bufferIndex;
  for( bufferIndex = 0; bufferIndex < count && samples.pop(sample); bufferIndex++) {
    sampleBuffer[bufferIndex] = ticksTo25ns(sample);
  }

  int encodedSize = encodeSampleBuffer(bufferIndex+1, outBuffer, sampleBuffer);

  output.println();
  output.write(outBuffer, encodedSize);
  output.println();
}

void PinSampler::processSampleBuffer() {
  if ( currentState == ERROR ) {
    output.printf("\nError: Buffer Overflow\n");
    stopSampling();
  } else if ( currentState == STOPPING_SAMPLING ) {
    stopSampling();
  } else if ( currentState == SAMPLING ) {
    // If there are enough samples in the buffer send them
    if ( samples.size() > NUMBER_OF_SAMPLES_IN_BATCH ) {
      sendOutputBuffer(NUMBER_OF_SAMPLES_IN_BATCH);
    }
  }
}

void PinSampler::processHalfDmaBuffer( PinSampler::BUFFER_HALF half ) {

  // Determine which half of the buffer to queue, this code doesn't
  // hardcoding the array size... 
  uint32_t *from, *to;
  size_t bufferSize = sizeof(dmaBuffer)/sizeof(uint32_t);
  switch(half) {
    case BOTTOM:
      from = &dmaBuffer[0];
      to = &dmaBuffer[bufferSize/2];
      break;
    case TOP:
      from = &dmaBuffer[bufferSize/2];
      to = &dmaBuffer[bufferSize];
      break;
  }

  // Queue the buffered samples for processing and in the process
  // convert from counter ticks to the number of ticks since the previous
  // capture.
  while(from < to )
  {
    uint32_t currentSample = *from++;
    uint32_t pulseWidth;

    // We need to detect when the counter rolls over and correct for this 
    // in the calculation.
    if ( currentSample < prevSample ) {
      pulseWidth = 0xFFFF + currentSample - prevSample;
    } else {
      pulseWidth = currentSample - prevSample;
    }

    if ( !samples.push(pulseWidth) ) {
      // We just dropped a sample...
      currentState = ERROR;
    }

    prevSample = currentSample;
  }

  checkForOverflow();
}


/* 
  FIXME: Yep this is broken non standard usuage of offsetof but it 
  happens to work with this toolchain. So shut up GCC ... of course 
  fixing the code so the compiler doesn't complain would be preferrable !
*/
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winvalid-offsetof"
PinSampler *PinSampler::getInstanceFromHdma(DMA_HandleTypeDef *hdma) {
   return reinterpret_cast<PinSampler *>(
            reinterpret_cast<char *>(hdma) 
            - offsetof(PinSampler, hdma));
}
#pragma GCC diagnostic pop

TIM_HandleTypeDef * PinSampler::getTimerHandleFromHdma(DMA_HandleTypeDef *hdma) {
  return reinterpret_cast<TIM_HandleTypeDef *>(hdma->Parent);
}

void PinSampler::checkForOverflow() {
    uint32_t tmpisr;

    typedef struct
    {
      __IO uint32_t ISR;   /*!< DMA interrupt status register */
      __IO uint32_t Reserved0;
      __IO uint32_t IFCR;  /*!< DMA interrupt flag clear register */
    } DMA_Base_Registers;
 
    /* calculate DMA base and stream number */
    DMA_Base_Registers *regs = (DMA_Base_Registers *)hdma.StreamBaseAddress;

    tmpisr = regs->ISR;

    if ( (tmpisr & (DMA_FLAG_TCIF0_4 << hdma.StreamIndex)) != RESET
        || (tmpisr & (DMA_FLAG_HTIF0_4 << hdma.StreamIndex)) != RESET
    ) {
      // We just dropped a sample...
      currentState = ERROR;
    }
 
}

void PinSampler::timerDmaCaptureComplete(DMA_HandleTypeDef *hdma) {
  getInstanceFromHdma(hdma)->processHalfDmaBuffer(TOP);
  getTimerHandleFromHdma(hdma)->Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;
}

void PinSampler::timerDmaCaptureHalfComplete(DMA_HandleTypeDef *hdma) {
  getInstanceFromHdma(hdma)->processHalfDmaBuffer(BOTTOM);
  getTimerHandleFromHdma(hdma)->Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;
}

void PinSampler::startSampling() {
  // Don't start sampling if we already are
  if ( currentState != IDLE )
      return;

  /* Start the counter */
  prevSample = samplerTimer.getCount(); // record the count at start
  samples.clear();
  
  processSampleBufferCallback->start();

  currentState = WAITING_FOR_INDEX;
  output.println("====== sampling started ======");
}

void PinSampler::indexHolePassing() {
  // If we are sampling we can stop the sampling since we've got the entire track.
  if (currentState == SAMPLING){
    //indexStop = indexTimer.getCaptureCompare(indexChannel);
    //indexPeriod = indexStop - indexStart; 
    if ( indexPeriod < 0 )
      indexPeriod += 0xffff;

    currentState = STOPPING_SAMPLING;
  } 
  // If we're waiting for the index hole, just enable the timer, this starts
  // the DMA based circular buffer.
  else if (currentState == WAITING_FOR_INDEX) {
    //indexStart = indexTimer.getCaptureCompare(indexChannel);
    currentState = SAMPLING;
    /* Enable the DMA stream */
    TIM_HandleTypeDef *halHandle = samplerTimer.getHandle();
    if (HAL_DMA_Start_IT(halHandle->hdma[TIM_DMA_ID_CC2], (uint32_t)&halHandle->Instance->CCR2, (uint32_t) &dmaBuffer, 100) != HAL_OK)
    {
      Error_Handler();
    }
    /* Enable the TIM Capture/Compare 2  DMA request */
    __HAL_TIM_ENABLE_DMA(halHandle, TIM_DMA_CC2);
    __HAL_TIM_ENABLE(halHandle);
  }

}

void PinSampler::stopSampling() {
  // Don't stop sampling unless we already are
  if ( ! (currentState == SAMPLING 
          || currentState == WAITING_FOR_INDEX 
          || currentState == STOPPING_SAMPLING 
          || currentState == ERROR ) )
      return;

  processSampleBufferCallback->stop();
  TIM_HandleTypeDef *halHandle = samplerTimer.getHandle();
  if (HAL_DMA_Abort(halHandle->hdma[TIM_DMA_ID_CC2]) != HAL_OK) {
    Error_Handler();
  }

  __HAL_TIM_DISABLE_DMA(samplerTimer.getHandle(), TIM_DMA_CC2);
  __HAL_TIM_DISABLE(samplerTimer.getHandle());

  TIM_CHANNEL_STATE_SET(samplerTimer.getHandle(), TIM_CHANNEL_2, HAL_TIM_CHANNEL_STATE_READY);
  TIM_CHANNEL_N_STATE_SET(samplerTimer.getHandle(), TIM_CHANNEL_2, HAL_TIM_CHANNEL_STATE_READY);

  while(!samples.isEmpty()) {
    sendOutputBuffer(NUMBER_OF_SAMPLES_IN_BATCH);
  }
  currentState = IDLE;
  double indexTimeMs = indexPeriod * indexTimer.getPrescaleFactor() / clockFrequency; 

  output.printf("====== sampling stopped (index time %0.3fms) ======\n", indexTimeMs);
}