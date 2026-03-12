#include "watchdog.hpp"
#include <ch.h>
#include <hal.h>

namespace {
  constexpr sysinterval_t watchdogRefreshPeriod = TIME_MS2I(500U);
  constexpr tprio_t watchdogThreadPriority = LOWPRIO;

  /*
   * LSI is nominally 32 kHz. With prescaler 64 the watchdog clock is 500 Hz,
   * so RLR=1000 gives a timeout of about 2 seconds.
   */
  static const WDGConfig wdgcfg = {
    .pr = STM32_IWDG_PR_64,
    .rlr = STM32_IWDG_RL(1000U),
#if STM32_IWDG_IS_WINDOWED
    .winr = STM32_IWDG_WIN_DISABLED,
#endif
  };

  bool resetByWatchdog = false;
  bool started = false;

  THD_WORKING_AREA(waWatchdog, 256U);

  void latchResetCause(void) __attribute__ ((constructor(101)));
  void latchResetCause(void)
  {
    const uint32_t csr = RCC->CSR;
    resetByWatchdog = (csr & RCC_CSR_IWDGRSTF) != 0U;
    RCC->CSR |= RCC_CSR_RMVF;
  }

  THD_FUNCTION(watchdogThread, arg)
  {
    (void)arg;
    chRegSetThreadName("watchdog");
    while (true) {
      chThdSleep(watchdogRefreshPeriod);
      wdgReset(&WDGD1);
    }
  }
}

bool Watchdog::wasReset(void)
{
  return resetByWatchdog;
}

void Watchdog::start(void)
{
  if (started) {
    return;
  }

  started = true;
  wdgStart(&WDGD1, &wdgcfg);
  chThdCreateStatic(waWatchdog, sizeof(waWatchdog), watchdogThreadPriority,
		    watchdogThread, nullptr);
}
