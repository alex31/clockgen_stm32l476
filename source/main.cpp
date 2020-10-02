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
  uint32_t freq;
  uint32_t mulExp;
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

ClockGenerator f1(&PWM_F1, CLOCK_F1_OUT_TIM_CH - 1), f2(&PWM_F2, CLOCK_F2_OUT_TIM_CH - 1);
Frequency frequencies[2] = {{1, 0, f1}, {1, 0, f2}};

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
  auto & [freq, mulExp, cg] = frequencies[ev.getIndex()];
  int32_t inc=0;

  switch (ev.getEvent()) {
  case  Events::Turn : {
    const int32_t delta = ev.getLoad();
    const int32_t sign = delta > 0 ? 1 : -1;
    // freq += std::clamp(delta*delta*delta, -200L, 200L);
    const int32_t deltabs = std::abs(delta);
    switch (deltabs) {
    case 0: break;
    case 1 :
    case 2 : inc = sign; break;
    case 3 : 
    case 4 : inc = 3*sign; break;
    default : inc  = sign * powf(deltabs*2, 2.0f); break;
    }

    const int magnitude = log10f(freq);
    const int nextMinMagnitude = std::clamp(magnitude-1, 0, 6);
    const int nextMaxMagnitude = std::clamp(magnitude+1, 0, 6);
    const uint32_t nextMin = powf(10.0, nextMinMagnitude);
    const uint32_t nextMax = powf(10.0, nextMaxMagnitude);
    freq += inc * powf(10, mulExp);
    freq -= freq % static_cast<uint32_t>(powf(10, mulExp));
    freq = std::clamp(freq, nextMin, nextMax);
    freq = std::clamp(freq, 1_hz, 999_khz);
    const int newMagnitude = log10f(freq);
    if (newMagnitude > 2)
      mulExp = (mulExp + newMagnitude - magnitude) % 4;
    else
      mulExp = 0;

    DebugTrace("magnitude = %d, nextMinMagnitude =%d, nextMaxMagnitude=%d\r\n"
	       "nextMin = %lu, nextMax=%lu, newFreq = %lu\n",
	       magnitude, nextMinMagnitude, nextMaxMagnitude,
	       nextMin, nextMax, freq);

    
    break;
  }
    
  case Events::ShortClick : {
    mulExp = ((mulExp + 1) % 4);
    if (mulExp == 0) {
      freq = 1;
    } else if (freq < 1000) {
      freq = 1000;
    } else {
      freq = powf(10, ceilf(log10f(freq+1)));
    }
    
    break;
  }

  default:
    DebugTrace("ignoring LongClick");
  }

  freq = std::clamp(freq, 1_hz, 999_khz);

  DebugTrace("mulExp=%ld", mulExp);

  if (freq < 1_khz)
    DebugTrace("freq[%u] = %03ld Hz", ev.getIndex(), freq);
  else if (freq < 10_khz)
    DebugTrace("freq[%u] = %04.2f KHz", ev.getIndex(), freq/1000.0f); 
  else if (freq < 100_khz)
    DebugTrace("freq[%u] = %04.1f KHz", ev.getIndex(), freq/1000.0f);
  else 
    DebugTrace("freq[%u] = %03ld KHz", ev.getIndex(), freq/1000); 
  
  cg.setFreq(freq);
}

