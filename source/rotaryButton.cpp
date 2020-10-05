#include "rotaryButton.hpp"

#include "event.hpp"
#include <functional>
#include <type_traits>


bool RotaryButton::init()
{
  return true;
}

bool RotaryButton::loop()
{
  auto [change, cnt] = encoder.getCnt();
 
  if (change) {
    int16_t delta = cnt - lastCnt;
    //    DebugTrace("delta = %d", delta);
    if (std::abs(delta) >= 2) {
      Event ev(Events::Turn, index, delta/2);
      lastCnt = cnt;
      chMBPostTimeout(&EVT::mb, ev.getEventAsMsg(), TIME_INFINITE);
    }
  }

  return true;
}

uint32_t RotaryButton::factoryIdx = 0U;
