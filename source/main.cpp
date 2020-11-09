#include <ch.h>
#include <hal.h>
#include "stdutil.h"		// necessaire pour initHeap
#ifndef NOSHELL
#include "ttyConsole.hpp"       // fichier d'entête du shell
#endif
#include "resetTTL.hpp"
#include "hardwareConf.hpp"
#include "beepIn.hpp"
#include "adc.hpp"
#include "ihm.hpp"
#include "freqCapture.hpp"
#include "fram.hpp"
#include "timeCount.hpp"
#include "storage.hpp"

   
void _init_chibios() __attribute__ ((constructor(101)));
void _init_chibios() {
  halInit();
  chSysInit();
  initHeap ();
#ifndef NOSHELL
  consoleInit();	// initialisation des objets liés au shell
#endif
  FRAM::init();
  ICU::init();
}
namespace std {
  void __throw_bad_function_call() { chSysHalt("throw_bad_function_call"); while(true);};
}

int main (void)
{
  TimeCount tc(NORMALPRIO);
  ADC &adc = ADC::instance(NORMALPRIO);
  BeepIn beepIn(NORMALPRIO);

  beepIn.run(TIME_MS2I(1));
  chThdSleepMilliseconds(10);
  adc.run(TIME_MS2I(100));
  Storage::instance().incPowerOn();
  tc.run(TIME_S2I(1));
  releaseResetAfter(TIME_S2I(1U)); // keep reset out value during 1 second
  IHM::init();

#ifndef NOSHELL
  consoleLaunch();  // lancement du shell
#endif
  
 
  chThdSleep(TIME_INFINITE);
}



