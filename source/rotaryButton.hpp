#pragma once
#include "workerClass.hpp"
#include "hardwareConf.hpp"
#include "encoderTimer.hpp"
#include "event.hpp"
#include <functional>
#include <type_traits>



namespace RB {
  constexpr size_t threadStackSize = 1024U;
}

class RotaryButton : public WorkerThread<RotaryButton> {
public:
  RotaryButton(const tprio_t m_prio, stm32_tim_t *timer) :
    WorkerThread("rotaryButton", RB::threadStackSize, m_prio),
    encoder(timer)
  {index = factoryIdx++;};
private:
  friend WorkerThread;
  bool init(void) final;
  bool loop(void) final;

  EncoderModeTimer encoder;
  uint16_t lastCnt=0;
  uint32_t index;
  static uint32_t factoryIdx;
};


