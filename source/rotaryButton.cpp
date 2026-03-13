#include "rotaryButton.hpp"

#include "event.hpp"
#include "pushButton.hpp"
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
    if (std::abs(delta) >= 2) {
      const bool turnWhilePress = PushButton::getState(index) != PBState::Up;
      const Events eventType = turnWhilePress ? Events::TurnPress : Events::Turn;
      const int16_t detents = delta / 2;
      const int32_t sign = detents > 0 ? 1 : -1;
      lastCnt = cnt;
      for (int16_t i = 0; i < std::abs(detents); i++) {
        Event ev(eventType, index, sign);
        chMBPostTimeout(&EVT::mb, ev.getEventAsMsg(), TIME_INFINITE);
      }
    }
  }

  return true;
}

uint32_t RotaryButton::factoryIdx = 0U;
