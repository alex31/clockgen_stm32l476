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

uint32_t ClockGenerator::quantizeFreq(uint32_t freq) const
{
  freq = std::clamp(freq, 1UL, 1_mhz);
  
  const uint32_t divider = pwmd->clock / freq;
  const uint16_t prescaler = 1 + (divider >> 16);
  const uint16_t reload = divider / prescaler;
  return ((STM32_SYSCLK / prescaler) / static_cast<uint32_t>(reload));
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

uint32_t ClockGenerator::searchNextReachableFreq(const uint32_t realizedFreq,
                                                 const bool up) const
{
  if (up) {
    if (realizedFreq >= 1_mhz) {
      return realizedFreq;
    }

    uint32_t lo = realizedFreq + 1U;
    uint32_t hi = std::min<uint32_t>(1_mhz, realizedFreq + 1U);
    while ((hi < 1_mhz) && (quantizeFreq(hi) <= realizedFreq)) {
      const uint32_t nextHi = std::min<uint32_t>(1_mhz, hi * 2U);
      if (nextHi == hi) {
        break;
      }
      hi = nextHi;
    }

    uint32_t best = realizedFreq;
    while (lo <= hi) {
      const uint32_t mid = lo + ((hi - lo) / 2U);
      const uint32_t quantized = quantizeFreq(mid);
      if (quantized > realizedFreq) {
        best = quantized;
        if (mid == 0U) {
          break;
        }
        hi = mid - 1U;
      } else {
        lo = mid + 1U;
      }
    }
    return best;
  }

  if (realizedFreq <= 1_hz) {
    return realizedFreq;
  }

  uint32_t lo = 1U;
  uint32_t hi = realizedFreq - 1U;
  uint32_t bestReq = lo;
  uint32_t bestFreq = realizedFreq;
  while (lo <= hi) {
    const uint32_t mid = lo + ((hi - lo) / 2U);
    const uint32_t quantized = quantizeFreq(mid);
    if (quantized < realizedFreq) {
      bestReq = mid;
      bestFreq = quantized;
      lo = mid + 1U;
    } else {
      if (mid == 0U) {
        break;
      }
      hi = mid - 1U;
    }
  }
  (void)bestReq;
  return bestFreq;
}

uint32_t ClockGenerator::nextReachableFreq(uint32_t fromFreq, const bool up,
                                           uint32_t stepCount) const
{
  uint32_t freq = quantizeFreq(fromFreq);
  stepCount = std::max<uint32_t>(1U, stepCount);
  while (stepCount-- != 0U) {
    const uint32_t nextFreq = searchNextReachableFreq(freq, up);
    if (nextFreq == freq) {
      break;
    }
    freq = nextFreq;
  }
  return freq;
}

void ClockGenerator::interpoledSetFreqProxy([[maybe_unused]] virtual_timer_t *vtl,
					    void *obj)
{
  ClockGenerator *cg = static_cast<ClockGenerator *>(obj);
  cg->interpoledSetFreq();
}
  
void ClockGenerator::interpoledSetFreq(void)
{
  const syssts_t sts = chSysGetStatusAndLockX();
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
  chSysRestoreStatusX(sts);
  
  play();
}
