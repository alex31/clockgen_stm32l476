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
	.dier = 0  
  };

class ClockGenerator {
public:
  ClockGenerator(PWMDriver * const _pwmd, const uint32_t _channel);
  [[nodiscard]] uint32_t setFreq(uint32_t freq);
  void pause(void);
  void play(void);

private:
  void start(void);
  PWMDriver * const pwmd;
  const uint32_t channel;
  PWMConfig pwmcfg{pwmcfgSkel};
};


