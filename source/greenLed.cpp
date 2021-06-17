#include "greenLed.hpp"
#include <ch.h>
#include <hal.h>

namespace {
  virtual_timer_t vt;
  void ledOnCb(ch_virtual_timer *vtl, void *arg);
  void ledOffCb(ch_virtual_timer *vtl, void *arg);
  uint32_t delay=0;
}

void blinkLed(uint32_t _delay)
{
  delay = _delay;
  chVTObjectInit(&vt);
  chVTSet(&vt, delay, &ledOffCb, nullptr);
}



namespace {
  void ledOnCb(ch_virtual_timer *vtl, [[maybe_unused]] void *arg)
  {
    chSysLockFromISR();
    palSetLine(LINE_LED_GREEN);
    chVTSetI(vtl, TIME_MS2I(100), &ledOffCb, nullptr);
    chSysUnlockFromISR();
  }
  
  void ledOffCb(ch_virtual_timer *vtl, [[maybe_unused]] void *arg)
  {
    chSysLockFromISR();
    palClearLine(LINE_LED_GREEN);
    chVTSetI(vtl, delay, &ledOnCb, nullptr);
    chSysUnlockFromISR();
  }
}


