#pragma once
#include "workerClass.hpp"
#include "hardwareConf.hpp"
#include "encoderTimer.hpp"
#include "event.hpp"
#include <functional>
#include <type_traits>



namespace RB {
  constexpr size_t threadStackSize = 1024U;
  uint32_t factoryIdx = 0U;
}

template<typename U>
class RotaryButton : public WorkerThread<RB::threadStackSize, RotaryButton<U>> {
public:
  RotaryButton(const tprio_t m_prio, stm32_tim_t *timer) :
    WorkerThread<RB::threadStackSize, RotaryButton>("rotaryButton", m_prio),
    encoder(timer)
  {index = RB::factoryIdx++;};
private:
  friend WorkerThread<RB::threadStackSize, RotaryButton>;
  bool init(void) final;
  bool loop(void) final;

  EncoderModeTimer encoder;
  uint16_t lastCnt=0;
  uint32_t index;
};

template<typename U>
bool RotaryButton<U>::init()
{
  return true;
}

template<typename U>
bool RotaryButton<U>::loop()
{
  auto [change, cnt] = encoder.getCnt();
 
  if (change) {
    int16_t delta = cnt - lastCnt;
    DebugTrace("delta = %d", delta);
    if (std::abs(delta) >= 2) {
      Event ev(Events::Turn, index, delta/2);
      lastCnt = cnt;
      chMBPostTimeout(&EVT::mb, ev.getEventAsMsg(), TIME_INFINITE);
    }
  }

  return true;
}

