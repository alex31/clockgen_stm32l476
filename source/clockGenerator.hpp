#pragma once

#include <ch.h>
#include <hal.h>
#include "hardwareConf.hpp"

static constexpr PWMConfig pwmcfgSkel = {     
	.frequency = 100_khz,
	.period    = 10_khz,
	.callback  = nullptr,
	.channels  = {
		      {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
		      {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
		      {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
		      {.mode = PWM_OUTPUT_DISABLED, .callback = NULL}
		      },
	.cr2  = 0, 
	.bdtr = 0,
	.dier = 0
  };

class ClockGenerator {
public:
  ClockGenerator(PWMDriver * const _pwmd, const uint32_t _channel);
  [[nodiscard]] uint32_t setFreq(uint32_t freq);
  [[nodiscard]] uint32_t quantizeFreq(uint32_t freq) const;
  [[nodiscard]] uint32_t nextReachableFreq(uint32_t fromFreq, bool up,
                                           uint32_t steps = 1U) const;
  void pause(void);
  void play(void);
  void enableOutput(bool en);
private:
  static constexpr int32_t steps = 100;
  static constexpr sysinterval_t interpolatedDelay = TIME_MS2I(500) /
    (steps + 1);
  void start(void);
  static void interpoledSetFreqProxy(ch_virtual_timer *vtl, void *obj);
  void interpoledSetFreq(void);
  [[nodiscard]] uint32_t searchNextReachableFreq(uint32_t realizedFreq,
                                                 bool up) const;

  PWMDriver * const pwmd;
  const uint32_t channel;
  PWMConfig pwmcfg{pwmcfgSkel};
  virtual_timer_t vt;
  float targetFreq;
  volatile float currentFreq;
  float incrementFreq;
};
