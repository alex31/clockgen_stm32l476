#include "encoderTimer.hpp"

void EncoderModeTimer::start(void)
{
  rccEnable();

  timer->PSC = 0;	    // prescaler must be set to zero
  timer->SMCR = 3;          // Encoder mode 3 : count on both TI1 and TI2
  timer->CCER = 0;          // rising edge polarity
  timer->ARR = 0xFFFFFFFF;  // count from 0-ARR or ARR-0
  timer->CCMR1 = 0xC1C1;    // f_DTS/16, N=8, IC1->TI1, IC2->TI2
  timer->CNT = 0;           // Initialize counter
  timer->EGR = 1;           // generate an update event
  timer->CR1 = 1;           // Enable the counter
}

bool EncoderModeTimer::cntIsUpdated(void)
{
  const bool change = timer->CNT == lastCnt;
  lastCnt = timer->CNT;
  return change;
}
void EncoderModeTimer::rccEnable(void)
{
  if (timer == STM32_TIM1) {
    rccEnableTIM1(NULL);
    rccResetTIM1();
  }
#ifdef TIM2
  else  if (timer == STM32_TIM2) {
    rccEnableTIM2(NULL);
    rccResetTIM2();
  }
#endif
#ifdef TIM3
  else  if (timer == STM32_TIM3) {
    rccEnableTIM3(NULL);
    rccResetTIM3();
  }
#endif
#ifdef TIM4
  else  if (timer == STM32_TIM4) {
    rccEnableTIM4(NULL);
    rccResetTIM4();
  }
#endif
#ifdef TIM5
  else  if (timer == STM32_TIM5) {
    rccEnableTIM5(NULL);
    rccResetTIM5();
  }
#endif
#ifdef TIM8
  else  if (timer == STM32_TIM8) {
    rccEnableTIM8(NULL);
    rccResetTIM8();
  }
#endif
#ifdef TIM9
  else  if (timer == STM32_TIM9) {
    rccEnableTIM9(NULL);
    rccResetTIM9();
  }
#endif
#ifdef TIM10
  else  if (timer == STM32_TIM10) {
    rccEnableTIM10(NULL);
    rccResetTIM10();
  }
#endif
#ifdef TIM11
  else  if (timer == STM32_TIM11) {
    rccEnableTIM11(NULL);
    rccResetTIM11();
  }
#endif
#ifdef TIM12
  else  if (timer == STM32_TIM12) {
    rccEnableTIM12(NULL);
    rccResetTIM12();
  }
#endif
#ifdef TIM13
  else  if (timer == STM32_TIM13) {
    rccEnableTIM13(NULL);
    rccResetTIM13();
  }
#endif
#ifdef TIM14
  else  if (timer == STM32_TIM14) {
    rccEnableTIM14(NULL);
    rccResetTIM14();
  }
#endif
#ifdef TIM15
  else  if (timer == STM32_TIM15) {
    rccEnableTIM15(NULL);
    rccResetTIM15();
  }
#endif
#ifdef TIM16
  else  if (timer == STM32_TIM16) {
    rccEnableTIM16(NULL);
    rccResetTIM16();
  }
#endif
#ifdef TIM17
  else  if (timer == STM32_TIM17) {
    rccEnableTIM17(NULL);
    rccResetTIM17();
  }
#endif
#ifdef TIM18
  else  if (timer == STM32_TIM18) {
    rccEnableTIM18(NULL);
    rccResetTIM18();
  }
#endif
#ifdef TIM19
  else  if (timer == STM32_TIM19) {
    rccEnableTIM19(NULL);
    rccResetTIM19();
  }
#endif
  else {
    chSysHalt("not a valid timer");
  }
};
