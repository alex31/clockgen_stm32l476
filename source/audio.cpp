#include <ch.h>
#include <hal.h>
#include <cstring>
#include <limits>
#include "stdutil.h"
#include "audio.hpp"
#include "cmath"





void Audio::start(void)
{
  gptStart(&GPTD6, &gpt6cfg1);
  dacStart(&DACD1, &dac1cfg1);
}

void Audio::select(const size_t loopIndex)
{
  if ((loopIndex != loop) &&
      (loopIndex <= loops.size())) {
    stopDac();
    loop = loopIndex;
    startDac();
  }
}

void Audio::select(const char* sampleName)
{
  auto sample = std::find_if (loops.begin(), loops.end(),
			      [&, sampleName] (const auto &s) {
				return s.name.compare(sampleName) == 0;
			      });
  if (sample != loops.end()) {
    const size_t index = sample - loops.begin();
    select(index);
  }
}

void Audio::play(void)
{
  if (not isPlaying())
    startTimer();
}

void Audio::pause(void)
{
  stopTimer();
}

void Audio::setDbVolume(const uint8_t volume)
{
  const uint32_t attn = 25 - (volume/4U);
  const float logAttn = powf(10, attn / 10.0f);
  setAttenuation(1.0f/logAttn);
}

void Audio::startDac(void)
{
  loopPtr = loops[loop].samples;
  loopLen = loops[loop].len;
  
  end_cb1(&DACD1);
  dacStartConversion(&DACD1, &dacgrpcfg1,
		     (dacsample_t *) dmaBuff, sizeof(dmaBuff)/sizeof(dmaBuff[0]));
}

void Audio::stopDac(void)
{
  dacStopConversion(&DACD1);
}


void Audio::startTimer(void)
{
  if (GPTD6.state == GPT_READY) {
    gptStopTimer(&GPTD6);
    gptStartContinuous(&GPTD6, GPT_COUNT);
  }
}

void Audio::stopTimer(void)
{
  if (GPTD6.state != GPT_READY)
    gptStopTimer(&GPTD6);
}

void Audio::end_cb1(DACDriver *dacp)
{
  // volatile size_t loopLen = 0;
  // volatile const dac8sample_t *loopPtr = nullptr;
  // const dac8sample_t dmaBuff[1024];
  // static constexpr halfBufferSize = sizeof(dmaBuff) / 2U;
  const source_dac_sample_t *loopUpperBound = loops[loop].samples + loops[loop].len;
  custom_dac_sample_t  *  const dmaBuf = (custom_dac_sample_t *) (dmaBuff + (dacIsBufferComplete(dacp) ?
						    halfBufferSize  :
						    0U));
  if ((loopPtr+halfBufferSize) < loopUpperBound) {
    audioCpy(dmaBuf, loopPtr, halfBufferSize);
    loopPtr += halfBufferSize;
    if (loopPtr == loopUpperBound)
      loopPtr = loops[loop].samples;
  } else {
    const size_t remain = loopUpperBound - loopPtr;
    audioCpy(dmaBuf, loopPtr, remain);
    const size_t remain2 = halfBufferSize - remain;
    audioCpy(dmaBuf+remain, loops[loop].samples, remain2);
    loopPtr = loops[loop].samples + remain2;
   }
}


void Audio::audioCpy(custom_dac_sample_t  *dest, const source_dac_sample_t *src, size_t n)
{
  for (size_t i=0; i<n; i++) {
    const float attenuated = 128.0f + ((float(src[i]) - 128.0f) * attenuation);
    dest[i] = static_cast<custom_dac_sample_t>(std::clamp(attenuated*16, 0.0f, 4095.0f));
  }
}




etl::string_view Audio::getName(const size_t index)
{
  size_t idx = (index >= loops.size()) ? loop : index;
    return etl::string_view(loops[idx].name);
}

const DACConfig Audio::dac1cfg1 = {
  .init         = 2047U,
  .datamode     = DAC_DHRM_12BIT_RIGHT,
  .cr           = 0,
  .mcr = 0
};

const DACConversionGroup Audio::dacgrpcfg1 = {
  .num_channels = 1U,
  .end_cb       = &Audio::end_cb1,
  .error_cb     = nullptr,
  .trigger      = DAC_TRG(0)
};

const GPTConfig Audio::gpt6cfg1 = {
   .frequency   = DAC_MAX_FREQUENCY,
  .callback     = NULL,
  .cr2          = TIM_CR2_MMS_1,    /* MMS = 010 = TRGO on Update Event.    */
  .dier         = 0U
};

size_t Audio::loop = std::numeric_limits<size_t>::max();
size_t Audio::loopLen = 0;
float Audio::attenuation = 1.0f;
const source_dac_sample_t *Audio::loopPtr = nullptr;
const custom_dac_sample_t IN_DMA_SECTION(Audio::dmaBuff[1024]) = {};
