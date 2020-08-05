#include <ch.h>
#include <hal.h>
#include <cstdio>
#include <cmath>
#include <utility>
#include <algorithm>
#include "etl/cstring.h"
#include "stdutil.h"		// necessaire pour initHeap
#ifndef NOSHELL
#include "ttyConsole.hpp"       // fichier d'entête du shell
#endif
#include "printf.h"
#include "resetTTL.hpp"
#include "clockGenerator.hpp"
#include "encoderTimer.hpp"
//#include "freqCapture.hpp"
//#include "display.hpp"


volatile uint32_t blinkWait=100;

static THD_WORKING_AREA(waBlinker, 304);	// declaration de la pile du thread blinker
static void blinker ([[maybe_unused]] void *arg)
{
  chRegSetThreadName("blinker");		
  
  while (true) {
    palToggleLine(LINE_LED_GREEN);
    chThdSleepMilliseconds(blinkWait);
  }
}
   
void _init_chibios() __attribute__ ((constructor(101)));
void _init_chibios() {
  halInit();
  chSysInit();
  initHeap ();
}

volatile int32_t mul = 1;
static virtual_timer_t vt;

int main (void)
{
  Clocks clocks;
  EncoderModeTimer bt1(STM32_TIM1);
  chVTObjectInit(&vt);
  
  palEnableLineEvent(LINE_BOUTON_F1_SW, PAL_EVENT_MODE_FALLING_EDGE);
  palSetLineCallback(LINE_BOUTON_F1_SW,
		     [] ([[maybe_unused]] void *arg) {
		       chSysLockFromISR();
		       if (not chVTIsArmedI(&vt)) {
			 chVTSetI(&vt, TIME_MS2I(100),
				  [] ([[maybe_unused]] void *arg) {
				    chSysLockFromISR();
				    if (palReadLine(LINE_BOUTON_F1_SW) == PAL_LOW) {
				      if (mul >= 1000)
					mul = 1;
				      else
					mul *= 10;
				    }
				    chSysUnlockFromISR();
				  }, nullptr);
		       }
		       chSysUnlockFromISR();
		     }, nullptr);
  
#ifndef NOSHELL
  consoleInit();	// initialisation des objets liés au shell
#endif
  releaseResetAfter(TIME_S2I(1U)); // keep reset out value during 1 second
  chThdCreateStatic(waBlinker, sizeof(waBlinker),
		    NORMALPRIO+2, &blinker, NULL);

  //  initFreqCapture();
  //  initDisplay();
#ifndef NOSHELL
  consoleLaunch();  // lancement du shell
#endif

  clocks.setMasterSlaveFreq(1000U, 2000U);
  clocks.setMasterSlaveFreq(10000U, 20000U);

  int32_t lastV=0;
  int32_t val =0;
  int32_t lastMul = mul;
  while(true) {
    auto [change, cnt] = bt1.getCnt();
    const int32_t scnt = cnt;
    
    if (change or (lastMul != mul)) {
      lastMul = mul;
      const int32_t delta = scnt - lastV;
      lastV = scnt;
      const int32_t sign = delta > 0 ? 1 : -1;
      // val += std::clamp(delta*delta*delta, -200L, 200L);
      
      switch (std::abs(delta)) {
      case 0: break;
      case 1 : val += sign; break;
      case 2 : val += 3*sign; break;
      default : val -= val % 10; val += 10*sign*(std::abs(delta)-1)*2; break;
      }
      
      if (val < 0)
	val += 1000;
      val %= 1000;

      int32_t hz = val * mul;
      if (mul == 1)
	DebugTrace("val = %03ld Hz", hz);
      else if (mul == 10)
	DebugTrace("val = %04.2f KHz", hz/1000.0f); 
      else if (mul == 100)
	DebugTrace("val = %04.1f KHz", hz/1000.0f);
      else 
	DebugTrace("val = %03ld KHz", hz/1000); 
      
    }
    chThdSleepMilliseconds(100);
  }
}

