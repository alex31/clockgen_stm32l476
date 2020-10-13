#pragma once
#include "workerClass.hpp"
#include "hardwareConf.hpp"


class BeepIn : public WorkerThread<BeepIn> {
public:
  BeepIn(const tprio_t m_prio) :
    WorkerThread("beep in", threadStackSize, m_prio)
  {};
private:
  static constexpr size_t threadStackSize = 1024U;
  friend WorkerThread<BeepIn>;
  bool init(void) final;
  bool loop(void) final;
};


