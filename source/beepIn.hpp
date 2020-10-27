#pragma once
#include "workerClass.hpp"
#include "hardwareConf.hpp"
#include "audio.hpp"


class BeepIn : public WorkerThread<BeepIn> {
public:
  BeepIn(const tprio_t m_prio) :
    WorkerThread("beep in", threadStackSize, m_prio)
  {};
  static Audio& getAudio(void) {return audio;}
private:
  static constexpr size_t threadStackSize = 1024U;
  static  Audio audio;
  static bool  lastState;
  friend WorkerThread<BeepIn>;
  bool init(void) final;
  bool loop(void) final;
};


