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
#include "rotaryButton.hpp"
#include "pushButton.hpp"
#include "hardwareConf.hpp"
//#include "freqCapture.hpp"
//#include "display.hpp"


volatile uint32_t blinkWait=100;
static void eventCb(const Event& ev);

struct Frequency {
  int32_t val;
  int32_t mul;
  ClockGenerator &cg;
};

struct Dummy1 {};
struct Dummy2 {};

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

namespace std {
  void __throw_bad_function_call() { chSysHalt("throw_bad_function_call"); while(true);};
}

volatile int32_t mul = 1;

ClockGenerator f1(&PWM_F1, CLOCK_F1_OUT_TIM_CH - 1), f2(&PWM_F2, CLOCK_F2_OUT_TIM_CH - 1);
Frequency frequencies[2] = {{0, 1, f1}, {0, 1, f2}};

int main (void)
{
  Event::init(&eventCb);
  RotaryButton<Dummy1> rb1(NORMALPRIO, ENCODER_F1);
  RotaryButton<Dummy2> rb2(NORMALPRIO, ENCODER_F2);
  PushButton<Dummy1> pb1(NORMALPRIO, LINE_BOUTON_F1_SW);
  PushButton<Dummy2> pb2(NORMALPRIO, LINE_BOUTON_F2_SW);
  
  // palSetLineCallback(LINE_BOUTON_F1_SW,
  // 		     [] ([[maybe_unused]] void *arg) {
  // 		       chSysLockFromISR();
  // 		       if (not chVTIsArmedI(&vt)) {
  // 			 chVTSetI(&vt, TIME_MS2I(100),
  // 				  [] ([[maybe_unused]] void *arg) {
  // 				    chSysLockFromISR();
  // 				    if (palReadLine(LINE_BOUTON_F1_SW) == PAL_LOW) {
  // 				      if (mul >= 1000)
  // 					mul = 1;
  // 				      else
  // 					mul *= 10;
  // 				    }
  // 				    chSysUnlockFromISR();
  // 				  }, nullptr);
  // 		       }
  // 		       chSysUnlockFromISR();
  // 		     }, nullptr);
  
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

  f1.setFreq(1000U);
  f2.setFreq(2000U);
  rb1.run(TIME_MS2I(100));
  rb2.run(TIME_MS2I(100));
  pb1.run(TIME_IMMEDIATE);
  pb2.run(TIME_IMMEDIATE);

 
  chThdSleep(TIME_INFINITE);
}



static void eventCb(const Event& ev)
{
  auto & [val, mul, cg] = frequencies[ev.getIndex()];


  switch (ev.getEvent()) {
  case  Events::Turn : {
    const int32_t delta = ev.getLoad();
    DebugTrace("delta =%ld", delta);
    const int32_t sign = delta > 0 ? 1 : -1;
    // val += std::clamp(delta*delta*delta, -200L, 200L);
    
    switch (std::abs(delta)) {
    case 0: break;
    case 1 : val += sign; break;
    case 2 : val += 3*sign; break;
    default : val -= val % 10; val += 20*sign*(std::abs(delta)-1); break;
    }
    
    if (val < 0)
      val += 1000;
    val %= 1000;
    break;
  }

  case Events::ShortClick : {
    if (mul >= 1000)
      mul = 1;
    else
      mul *= 10;
    break;
  }

  default:
    DebugTrace("ignoring LongClick");
  }
  
  
  int32_t hz = val * mul;
  if (mul == 1)
    DebugTrace("val[%u] = %03ld Hz", ev.getIndex(), hz);
  else if (mul == 10)
    DebugTrace("val[%u] = %04.2f KHz", ev.getIndex(), hz/1000.0f); 
  else if (mul == 100)
    DebugTrace("val[%u] = %04.1f KHz", ev.getIndex(), hz/1000.0f);
  else 
    DebugTrace("val[%u] = %03ld KHz", ev.getIndex(), hz/1000); 
  
  cg.setFreq(hz);
}

