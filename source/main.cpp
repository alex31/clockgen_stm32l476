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
#include "lcdDisplay.hpp"


volatile uint32_t blinkWait=100;
static void eventCb(const Event& ev);

struct Frequency {
  uint32_t lastFreq;
  uint32_t freq;
  ClockGenerator &cg;
};

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
Frequency frequencies[2] = {{1, 1, f1}, {1, 1, f2}};
LCDDisplay lcd(NORMALPRIO);

int main (void)
{
  Event::init(&eventCb);
  RotaryButton rb1(NORMALPRIO, ENCODER_F1);
  RotaryButton rb2(NORMALPRIO, ENCODER_F2);
  PushButton pb1(NORMALPRIO, LINE_BOUTON_F1_SW);
  PushButton pb2(NORMALPRIO, LINE_BOUTON_F2_SW);

  
  
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

  lcd.run(TIME_MS2I(500));
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
  auto & [lastFreq, freq, cg] = frequencies[ev.getIndex()];
  int32_t inc=0;
  static char buf[80];

  const uint32_t mulExp = std::clamp(static_cast<uint32_t>(log10f(freq)), 2UL, 5UL) -2UL;
 
  switch (ev.getEvent()) {
  case  Events::Turn : {
    const int32_t delta = ev.getLoad();
    const int32_t sign = delta > 0 ? 1 : -1;
    const int32_t deltabs = std::abs(delta);

    lastFreq = freq;

    switch (deltabs) {
    case 0: break;
    case 1 :
    case 2 : inc = sign; break;
    case 3 : 
    case 4 : inc = 3*sign; break;
    default : inc  = sign * powf((deltabs-3)*2, 1.0f+(deltabs/10.0f)); break;
    }

    const uint32_t minFreqInRange = powf(10.0f, floorf(log10f(freq))) -1.0f;
    inc = std::clamp(inc, -200L, 200L);
    freq += inc * powf(10, mulExp);
    freq = std::clamp(freq, minFreqInRange, 999_khz);
    freq -= freq % static_cast<uint32_t>(powf(10, mulExp));
    break;
  }
    
  case Events::ShortClick : {
    const enum class Direction {Up, Down} dir = lastFreq < freq ? Direction::Down :
							      Direction::Up;
    lastFreq = freq;
    if (dir == Direction::Up) {
      if (mulExp == 3) {
	freq = 1;
      } else if (freq < 1000) {
	freq = 1000;
      } else {
	freq = powf(10, ceilf(log10f(freq+1)));
      }
    } else { // dir == Direction::Down
     if (mulExp == 1) {
	freq = 100_khz;
      } else if (freq < 1000) {
	freq = 1000;
      } else {
	freq = powf(10, ceilf(log10f(freq-1)));
      }
    }
    
    break;
  }

  case Events::LongClick : {
    DebugTrace("ignoring Long Click");
    break;
  }
    
 case Events::DoubleClick : {
    DebugTrace("Double Click : undo");
    freq = lastFreq;
    break;
  }

  default : break;
  }

  freq = std::clamp(freq, 1_hz, 999_khz);

  //  DebugTrace("mulExp=%ld", mulExp);
  
  if (freq < 1_khz) {
    snprintf(buf, sizeof(buf), "freq[%u] = %03ld Hz", ev.getIndex(), freq);
  } else if (freq < 10_khz) {
    snprintf(buf, sizeof(buf), "freq[%u] = %04.2f KHz", ev.getIndex(), freq/1000.0f); 
  } else if (freq < 100_khz) {
    snprintf(buf, sizeof(buf), "freq[%u] = %04.1f KHz", ev.getIndex(), freq/1000.0f);
  } else  {
    snprintf(buf, sizeof(buf), "freq[%u] = %03ld KHz", ev.getIndex(), freq/1000); 
  }
  DebugTrace("%s", buf);
  lcd.write(3, etl::string_view(buf, sizeof(buf)));
  // lcd.write(ev.getIndex(), 0, "FRAQ"); // to test the library
  lcd.draw();
  lcd.enableCursor(freq % 2);
  lcd.setCursorPos(0, 10);
  cg.setFreq(freq);
}

