#pragma once

#include <ch.h>
#include <hal.h>
#include <utility>

class EncoderModeTimer {
public:
  EncoderModeTimer(stm32_tim_t *_timer) : timer(_timer) {start();}
  std::pair<bool, uint16_t> getCnt(void) {
    return {cntIsUpdated(), timer->CNT};
  }
private:
  void start(void);
  void rccEnable(void);
  bool cntIsUpdated(void);
  
  stm32_tim_t * const timer;
  uint16_t lastCnt=0U;
};
