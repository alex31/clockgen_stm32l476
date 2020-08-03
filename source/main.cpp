#include <ch.h>
#include <hal.h>
#include <cstdio>
#include <cmath>
#include <utility>
#include "etl/cstring.h"
#include "stdutil.h"		// necessaire pour initHeap
#ifndef NOSHELL
#include "ttyConsole.hpp"       // fichier d'entête du shell
#endif
#include "printf.h"
#include "resetTTL.hpp"
#include "clockGenerator.hpp"
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

int main (void)
{
  ClockGenerator cgm(&PWMD3, TimerMode::Master);
  TIM3->CR1 = 0;
  ClockGenerator cgs(&PWMD4, TimerMode::Slave);
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
  cgs.start();
  cgm.start();
   // for (uint32_t i=1; i<= 100000; i++) {
   //   cg.setFreq(i);
   // }
  cgs.setFreq(2000);
  cgm.setFreq(1000);
  blinkWait = 500;
  // main thread does nothing
  chThdSleep(TIME_INFINITE);
}


