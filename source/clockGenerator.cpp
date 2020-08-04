#include "clockGenerator.hpp"
#include "stdutil.h"
#include <algorithm>

namespace {

  constexpr uint32_t channel = 0U;
}
ClockGenerator::ClockGenerator(PWMDriver * const _pwmd, const TimerMode _mode) :
    pwmd(_pwmd), mode(_mode)
{
  if (mode == TimerMode::Master) {
    pwmcfg.cr2 = 0b001 << TIM_CR2_MMS_Pos;
 }

  start();
}

void ClockGenerator::start(void)
{
  pwmStart(pwmd, &pwmcfg);
  pwmd->tim->CR1 &= ~STM32_TIM_CR1_CEN;

  if (mode == TimerMode::Master) {
    pwmd->tim->SMCR = 1U << TIM_SMCR_MSM_Pos;
  } 
}

void ClockGenerator::setFreq(uint32_t freq)
{
  freq = std::clamp(freq, 1UL, 100000UL);

  if (mode == TimerMode::Slave) {
    pwmd->tim->SMCR = 0;
  }
  pwmd->tim->CR1 &= ~STM32_TIM_CR1_CEN;
  
  uint32_t divider = pwmd->clock / freq;
  
  uint16_t prescaler = 1 + (divider >> 16);
  uint16_t reload = divider / prescaler;

  if (reload < 4) {
    DebugTrace("f=%lu prescaler=%u reload=%u width=%u",
	       freq, prescaler, reload, reload / 2);
  }
  
  pwmd->tim->PSC = prescaler - 1;
  pwmd->tim->ARR = reload - 1;

  pwmEnableChannel(pwmd, channel, (reload / 2));

  if (mode == TimerMode::Master) {
   pwmd->tim->CR1 |= STM32_TIM_CR1_CEN;
  } else {
    pwmd->tim->SMCR = 
      // TIM4 slave triggered by TIM3
      (0b010 << TIM_SMCR_TS_Pos) |
      // TIM4 Enable controled by TIM3 enable
      (0b110 << TIM_SMCR_SMS_Pos);
  }

}
