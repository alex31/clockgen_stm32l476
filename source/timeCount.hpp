#pragma once
#include "workerClass.hpp"



class TimeCount : public WorkerThread<TimeCount> {
public:
  TimeCount(const tprio_t m_prio) :
    WorkerThread("timeCount", threadStackSize, m_prio)
  {};
private:
  static constexpr size_t threadStackSize = 1024U;
  friend WorkerThread<TimeCount>;
  bool init(void) final;
  bool loop(void) final;
};


