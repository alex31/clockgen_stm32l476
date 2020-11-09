#pragma once
#include "workerClass.hpp"
#include "hardwareConf.hpp"
#include <array>

namespace PB {
  constexpr size_t threadStackSize = 1024U;
}

enum class PBState {Up, Down, LongDown, DoubleClick};

class PushButton : public WorkerThread<PushButton> {
public:
  PushButton(const tprio_t m_prio, const ioline_t _line) :
    WorkerThread("pushButton", PB::threadStackSize, m_prio),
    line(_line)
  { index = factoryIdx++; buttons[index] = this;};
  static  PBState getState(const uint8_t index) {
    return buttons[index]->state;
  }
private:
  friend WorkerThread<PushButton>;
  bool init(void) final;
  bool loop(void) final;
  void proceedUp(void);
  void proceedDown(void);
  
  ioline_t line;
  //  std::function<void(Event&)> cb;
  uint32_t index;
  virtual_timer_t vt;
  PBState state = PBState::Up;
  systime_t ts = chVTGetSystemTimeX();
  static uint32_t factoryIdx;
  static  std::array<PushButton*, 2> buttons;
};


