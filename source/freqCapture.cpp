#include "freqCapture.hpp"
#include "event.hpp"
#include "hardwareConf.hpp"
#include "stdutil.h"


namespace {
  constexpr uint32_t ICU_FREQ = STM32_SYSCLK;
  bool overflow = false;
  const ICUConfig icucfg = {
  .mode = ICU_INPUT_ACTIVE_HIGH,
  .frequency = ICU_FREQ,                                
  .width_cb = nullptr,
  .period_cb = [] ([[maybe_unused]] ICUDriver *icup) {overflow = false;},
  .overflow_cb = [] ([[maybe_unused]] ICUDriver *icup) {overflow = true;},
  .channel = static_cast<icuchannel_t>(FREQMETER_IN_TIM_CH -1U),
  .dier = 0,
  .arr = STM32_TIMCLK1   // overflow to detect unactivity  after 1 second
};

}

namespace ICU {
  
void init(void)
{
  icuStart(&ICU_IN, &icucfg);
  icuStartCapture(&ICU_IN);
  icuEnableNotifications(&ICU_IN);
}


float getFrequency(void)
{
  if (overflow) {
    return 0.0f;
  } else {
    return static_cast<float>(ICU_FREQ) / icuGetPeriodX(&ICU_IN);    
  }
}

float getDuty(void)
{
  if (overflow) {
    return 0.0f;
  }
  
  chSysLock();
  const uint32_t w = icuGetWidthX(&ICU_IN);
  const uint32_t p = icuGetPeriodX(&ICU_IN);
  chSysUnlock();
  return static_cast<float>(w) / p;
}

uint32_t getPulseWidthUsec(void)
{
  if (overflow) {
    return 0U;
  }
  const uint32_t w = icuGetWidthX(&ICU_IN);
  return RTC2US(STM32_SYSCLK, w);
}

}
