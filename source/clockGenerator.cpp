#include "clockGenerator.hpp"
#include "stdutil.h"
#include <algorithm>

ClockGenerator::ClockGenerator(PWMDriver * const _pwmd, const uint32_t _channel) :
    pwmd(_pwmd), channel(_channel)
{
  pwmcfg.channels[channel].mode = PWM_OUTPUT_ACTIVE_HIGH;
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

void ClockGenerator::start(void)
{
  pwmStart(pwmd, &pwmcfg);
  pause();
}

uint32_t ClockGenerator::setFreq(uint32_t freq)
{
  freq = std::clamp(freq, 1UL, 1_mhz);
  
  pause();
  
  uint32_t divider = pwmd->clock / freq;
  uint16_t prescaler = 1 + (divider >> 16);
  uint16_t reload = divider / prescaler;

  if (reload < 4) {
    DebugTrace("WARNING f=%lu prescaler=%u reload=%u width=%u",
	       freq, prescaler, reload, reload / 2);
  } 

  pwmd->tim->PSC = prescaler - 1;
  pwmd->tim->ARR = reload-1;
  pwmEnableChannel(pwmd, channel, (reload / 2));
  play();
  return ((STM32_SYSCLK / prescaler) / reload);
}
