#include "freqCapture.hpp"
#include "event.hpp"
#include "hardwareConf.hpp"
#include "stdutil.h"
#include <limits>
#include <cmath>


namespace {
  /* Input filter, see RM0351 page 1070 on IC1F IC2F on CCMR1 timer register*/
  enum ICFFilter_t {ICF_noFilter=0,
		    ICF_fCKINT_N2, ICF_fCKINT_N4, ICF_fCKINT_N8,
		    ICF_fDTS_Div2_N6, ICF_fDTS_Div2_N8,
		    ICF_fDTS_Div4_N6, ICF_fDTS_Div4_N8,
		    ICF_fDTS_Div8_N6, ICF_fDTS_Div8_N8,
		    ICF_fDTS_Div16_N5, ICF_fDTS_Div16_N6, ICF_fDTS_Div16_N8,
		    ICF_fDTS_Div32_N5, ICF_fDTS_Div32_N6, ICF_fDTS_Div32_N8, ICF_End};
  static_assert (ICF_End == 16U);

  constexpr uint32_t ICU_FREQ = STM32_SYSCLK;

  // use the most effective value, limiting input frequency to 100khz
  constexpr uint32_t ICxF_Filter = ICF_fDTS_Div32_N8;
  
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
  // enable glitch digital filter  
  ICU_IN.tim->CCMR1 |= ((ICxF_Filter << TIM_CCMR1_IC1F_Pos) |
			(ICxF_Filter << TIM_CCMR1_IC2F_Pos)); 
  icuStartCapture(&ICU_IN);
}


float getFrequency(void)
{
  if (overflow()) {
    return 0.0f;
  } else {
    //    DebugTrace("icuGetPeriodX = %lu", icuGetPeriodX(&ICU_IN));
    return floorf(static_cast<float>(ICU_FREQ) / (icuGetPeriodX(&ICU_IN)+1U));
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
