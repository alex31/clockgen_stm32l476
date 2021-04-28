#include "freqCapture.hpp"
#include "event.hpp"
#include "hardwareConf.hpp"
#include "stdutil.h"
#include <limits>

namespace {
  constexpr uint32_t ICU_FREQ = STM32_SYSCLK;

  
  const ICUConfig icucfg = {
  .mode = ICU_INPUT_ACTIVE_HIGH,
  .frequency = ICU_FREQ,                                
  .width_cb = nullptr,
  .period_cb = nullptr,
  .overflow_cb = nullptr,
  .channel = static_cast<icuchannel_t>(FREQMETER_IN_TIM_CH -1U),
  .dier = 0,
  .arr =  std::numeric_limits<uint32_t>::max()
};

  bool overflow(void);
}

namespace ICU {
  
void init(void)
{
  icuStart(&ICU_IN, &icucfg);
  icuStartCapture(&ICU_IN);
}


float getFrequency(void)
{
  if (overflow()) {
    return 0.0f;
  } else {
    //    DebugTrace("icuGetPeriodX = %lu", icuGetPeriodX(&ICU_IN));
    return static_cast<float>(ICU_FREQ) / (icuGetPeriodX(&ICU_IN)+1U);
  }
}

float getDuty(void)
{
  chSysLock();
  float res = 0.0f;
  if (not overflow()) {
    const uint32_t w = icuGetWidthX(&ICU_IN);
    const uint32_t p = icuGetPeriodX(&ICU_IN);
    res = static_cast<float>(w) / p;
  }
  chSysUnlock();
  return res;
}

uint32_t getPulseWidthUsec(void)
{
  if (overflow()) {
    return 0U;
  }
  const uint32_t w = icuGetWidthX(&ICU_IN);
  return RTC2US(STM32_SYSCLK, w);
}

}



namespace {
  bool overflow(void) {
    if (ICU_IN.tim->CNT > STM32_SYSCLK) {
      ICU_IN.tim->CNT =  STM32_SYSCLK +1;
      return true;
    } else {
      return false;
    }
  }
}
