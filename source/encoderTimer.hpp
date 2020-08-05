#pragma once

#include <ch.h>
#include <hal.h>
#include <utility>

class EncoderModeTimer {
public:
  EncoderModeTimer(stm32_tim_t *_timer) : timer(_timer) {start();}
  std::pair<bool, uint32_t> getCnt(void) {
    return {cntIsUpdated(), timer->CNT >> 1U};
  }
private:
  void start(void);
  void rccEnable(void);
  bool cntIsUpdated(void);
  
  stm32_tim_t * const timer;
  uint32_t lastCnt=0U;
};
