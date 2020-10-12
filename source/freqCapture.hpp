#pragma once
#include "ch.h" 
#include "hal.h" 

namespace ICU {
  void init(void) ;
  float getFrequency(void);
  float getDuty(void);
  uint32_t getPulseWidthUsec(void);
}
