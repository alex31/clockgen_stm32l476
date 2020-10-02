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
    // result of substract from 2 unsigned must be done in signed context
    // so the cast is made necessary because of the "/ 2"
    int16_t delta = static_cast<int16_t>(cnt - lastCnt) / 2;
    lastCnt = cnt;
    // val += std::clamp(delta*delta*delta, -200L, 200L);
     
    Event ev(Events::Turn, index, delta);
    chMBPostTimeout(&EVT::mb, ev.getEventAsMsg(), TIME_INFINITE);
  }

  return true;
}

