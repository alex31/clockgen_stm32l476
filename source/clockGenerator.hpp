#pragma once

#include <ch.h>
#include <hal.h>

enum class TimerMode{Slave, Master};

static constexpr PWMConfig pwmcfgSkel = {     
	.frequency = 100000,
	.period    = 10000,
	.callback  = nullptr,
	.channels  = {
		      // sortie active, polarité normale, pas de callback
		      {.mode = PWM_OUTPUT_ACTIVE_HIGH, .callback = NULL},
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
  ClockGenerator(PWMDriver * const _pwmd, const TimerMode _mode);
  void start(void);
  void setFreq(uint32_t freq);
  void setMode(const TimerMode mode);
private:
  
  PWMDriver * const pwmd;
  const TimerMode mode;
  PWMConfig pwmcfg = pwmcfgSkel;
};
