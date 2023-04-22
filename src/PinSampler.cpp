#include "PinSampler.h"
#include <base64.hpp>

DMA_HandleTypeDef* DMA1_Stream6_hdma;

extern "C" {
  void DMA1_Stream6_IRQHandler(void)
  {
    HAL_DMA_IRQHandler(DMA1_Stream6_hdma);
  }
}

PinSampler::PinSampler(USBSerial& output, MultiTask& multitask, const uint8_t pin) 
    : output(output), multitask(multitask), pin(pin), sampleBufferOverflow(false) {
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

    drainCallback = multitask.every(0, std::bind(&PinSampler::drainSampleBuffer,this), false);
 }

void PinSampler::drainSampleBuffer() {
  const int NUM_SAMPLES = 24;
  uint8_t outBuffer[NUM_SAMPLES*4];
  uint8_t base64Buffer[encode_base64_length(NUM_SAMPLES*4)+1];

  // Check for overflow condition
  if ( sampleBufferOverflow ) {
    output.printf("\nError: Buffer Overflow\n");
    stopSampling();
    return;
  }

  if ( samples.size() > NUM_SAMPLES ) {
    uint32_t sample;
   
    // Multi byte decoding:
    //   0. set c = 0
    //   1. read byte b, c = c + (b & 7F)
    //   2. if b&80 == 1, goto 1
    int p = 0;
    for( int i = 0; i < NUM_SAMPLES && samples.pop(sample); i++) {
      sample = sample >> 2; // discard first 2 bits - we don't need the resolution

      // While the sample has more bits keep ading bytes to the output buffer.
      // These bytes have the most sigificnat bit set to indicate more bytes to 
      // follow.
      while(sample > 0x7F) {
        outBuffer[p++] = 0x80 & (sample & 0x7F); 
        sample = sample >> 7;
      }

      // The last byte has most significant bit clear to indicate no more bytes 
      // to follow.
      outBuffer[p++] = sample; 
    }

    size_t encodedSize = encode_base64(outBuffer, p, base64Buffer);
    output.write(base64Buffer, encodedSize);
    output.println();
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
      sampleBufferOverflow = true;
    }

    prevSample = currentSample;
  }
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

void PinSampler::timerDmaCaptureComplete(DMA_HandleTypeDef *hdma) {
  getInstanceFromHdma(hdma)->processHalfDmaBuffer(TOP);
  getTimerHandleFromHdma(hdma)->Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;
}

void PinSampler::timerDmaCaptureHalfComplete(DMA_HandleTypeDef *hdma) {
  getInstanceFromHdma(hdma)->processHalfDmaBuffer(BOTTOM);
  getTimerHandleFromHdma(hdma)->Channel = HAL_TIM_ACTIVE_CHANNEL_CLEARED;
}

void PinSampler::startSampling() {
  TIM_HandleTypeDef *halHandle = timer.getHandle();
  int halChannel = timer.getChannel(channel); 

  TIM_CHANNEL_STATE_SET(halHandle, halChannel, HAL_TIM_CHANNEL_STATE_BUSY);
  TIM_CHANNEL_N_STATE_SET(halHandle, halChannel, HAL_TIM_CHANNEL_STATE_BUSY);

  TIM_CCxChannelCmd(halHandle->Instance, halChannel, TIM_CCx_ENABLE);

  /* Set the DMA capture callbacks */
  halHandle->hdma[TIM_DMA_ID_CC2]->XferCpltCallback = &PinSampler::timerDmaCaptureComplete;
  halHandle->hdma[TIM_DMA_ID_CC2]->XferHalfCpltCallback = &PinSampler::timerDmaCaptureHalfComplete;;

  /* Set the DMA error callback */
  halHandle->hdma[TIM_DMA_ID_CC2]->XferErrorCallback = TIM_DMAError ;

  /* Enable the DMA stream */
  if (HAL_DMA_Start_IT(halHandle->hdma[TIM_DMA_ID_CC2], (uint32_t)&halHandle->Instance->CCR2, (uint32_t) &dmaBuffer, 100) != HAL_OK)
  {
    Error_Handler();
  }
  /* Enable the TIM Capture/Compare 2  DMA request */
  __HAL_TIM_ENABLE_DMA(halHandle, TIM_DMA_CC2);

  /* Start the counter */
  prevSample = timer.getCount(); // record the count at start
  sampleBufferOverflow = false;
  samples.clear();
  __HAL_TIM_ENABLE(halHandle);

  drainCallback->start();
}

void PinSampler::stopSampling() {
  drainCallback->stop();
  TIM_HandleTypeDef *halHandle = timer.getHandle();
  if (HAL_DMA_Abort(halHandle->hdma[TIM_DMA_ID_CC2]) != HAL_OK) {
    Error_Handler();
  }

  __HAL_TIM_DISABLE_DMA(timer.getHandle(), TIM_DMA_CC2);
  __HAL_TIM_DISABLE(timer.getHandle());

  TIM_CHANNEL_STATE_SET(timer.getHandle(), TIM_CHANNEL_2, HAL_TIM_CHANNEL_STATE_READY);
  TIM_CHANNEL_N_STATE_SET(timer.getHandle(), TIM_CHANNEL_2, HAL_TIM_CHANNEL_STATE_READY);

}