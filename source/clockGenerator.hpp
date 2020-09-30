#pragma once

#include <ch.h>
#include <hal.h>
#include "hardwareConf.hpp"

static constexpr PWMConfig pwmcfgSkel = {     
	.frequency = 100_khz,
	.period    = 10_khz,
	.callback  = nullptr,
	.channels  = {
		      // sortie active, polarité normale, pas de callback
		      {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
		      // sortie inactive
		      {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
		      // sortie inactive
		      {.mode = PWM_OUTPUT_DISABLED, .callback = NULL},
		      // sortie inactive
		      {.mode = PWM_OUTPUT_DISABLED, .callback = NULL}
		      },
	.cr2  = 0, // doit être initialisé à 0 (voir stm32f4 reference manuel)
	.dier = 0  // doit être initialisé à 0 (voir stm32f4 reference manuel)
  };

class ClockGenerator {
public:
  ClockGenerator(PWMDriver * const _pwmd, const uint32_t _channel);
  void setFreq(uint32_t freq);
  void pause(void);
  void play(void);

private:
  void start(void);
  PWMDriver * const pwmd;
  const uint32_t channel;
  PWMConfig pwmcfg{pwmcfgSkel};
};


