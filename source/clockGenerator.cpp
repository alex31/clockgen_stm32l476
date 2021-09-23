#include "clockGenerator.hpp"
#include "stdutil.h"
#include <algorithm>

ClockGenerator::ClockGenerator(PWMDriver * const _pwmd, const uint32_t _channel) :
    pwmd(_pwmd), channel(_channel)
{
  pwmcfg.channels[channel].mode = PWM_OUTPUT_ACTIVE_HIGH;
  targetFreq = currentFreq = incrementFreq = 0;
  chVTObjectInit(&vt);
  start();
}

void ClockGenerator::pause(void)
{
  pwmd->tim->CR1 &= ~STM32_TIM_CR1_CEN;
}

void ClockGenerator::play(void)
{
  pwmd->tim->CR1 |= STM32_TIM_CR1_CEN;
}

void ClockGenerator::enableOutput(bool en)
{
  pwmEnableChannelOutput(pwmd, 1, en); // works because F1 and F2 output on channel 2
  if (en)
    interpoledSetFreq();
}

void ClockGenerator::start(void)
{
  pwmStart(pwmd, &pwmcfg);
  pause();
}

uint32_t ClockGenerator::setFreq(uint32_t freq)
{
  freq = std::clamp(freq, 1UL, 1_mhz);
  
  const uint32_t divider = pwmd->clock / freq;
  const uint16_t prescaler = 1 + (divider >> 16);
  const uint16_t reload = divider / prescaler;

  if (reload < 4) {
    DebugTrace("WARNING f=%lu prescaler=%u reload=%u width=%u",
	       freq, prescaler, reload, reload / 2);
  } 
  targetFreq = ((STM32_SYSCLK / prescaler) / static_cast<float>(reload));
  incrementFreq = (targetFreq - currentFreq) / steps;
  chVTReset(&vt);
  chVTSet(&vt, interpolatedDelay,  &interpoledSetFreqProxy, this);
  return targetFreq;
}

void ClockGenerator::interpoledSetFreqProxy([[maybe_unused]] ch_virtual_timer *vtl,
					    void *obj)
{
  ClockGenerator *cg = static_cast<ClockGenerator *>(obj);
  cg->interpoledSetFreq();
}
  
void ClockGenerator::interpoledSetFreq(void)
{
  chSysLockFromISR();
  if (incrementFreq > 0.0f) {
    currentFreq = std::min(currentFreq + incrementFreq,
			   targetFreq);
  } else {
    currentFreq = std::max(currentFreq + incrementFreq,
			   targetFreq);
  }
  if (static_cast<uint32_t>(currentFreq) != static_cast<uint32_t>(targetFreq)) {
    chVTSetI(&vt, interpolatedDelay,  &interpoledSetFreqProxy, this);
  }
  pause();
  
  uint32_t divider = pwmd->clock / static_cast<uint32_t>(currentFreq);
  uint16_t prescaler = 1 + (divider >> 16);
  uint16_t reload = divider / prescaler;
  
  pwmd->tim->PSC = prescaler - 1;
  pwmd->tim->ARR = reload - 1;
  pwmEnableChannelI(pwmd, channel, (reload / 2));
  chSysUnlockFromISR();
  
  play();
}
