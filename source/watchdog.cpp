#include "watchdog.hpp"
#include <ch.h>
#include <hal.h>

namespace {
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

  void latchResetCause(void) __attribute__ ((constructor(101)));
  void latchResetCause(void)
  {
    const uint32_t csr = RCC->CSR;
    resetByWatchdog = (csr & RCC_CSR_IWDGRSTF) != 0U;
    RCC->CSR |= RCC_CSR_RMVF;
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
}

void Watchdog::reload(void)
{
  if (started) {
    wdgReset(&WDGD1);
  }
}
