#pragma once
#include "workerClass.hpp"
#include "hardwareConf.hpp"


class AUDIO : public WorkerThread<AUDIO> {
public:
  AUDIO(const tprio_t m_prio) :
    WorkerThread("dac audio", threadStackSize, m_prio)
  {};
private:
  friend WorkerThread<AUDIO>;
  bool init(void) final;
  bool loop(void) final;
  static constexpr size_t threadStackSize = 1024U;
};


