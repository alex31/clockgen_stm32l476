#include <ch.h>
#include <hal.h>
#include <cstdio>
#include <cmath>
#include <utility>
#include <algorithm>
#include "etl/string.h"
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
#include "adc.hpp"
#include "freqCapture.hpp"
#include "fram.hpp"
#include "beepIn.hpp"

volatile uint32_t blinkWait=100;
static void eventCb(const Event& ev);

enum class Direction {Up=0x7E, Down=0x7F};
struct Frequency {
  uint32_t lastFreq;
  uint32_t freq;
  ClockGenerator &cg;
  Direction dir;
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
Frequency frequencies[2] = {{1, 1, f1, Direction::Up}, {1, 1, f2, Direction::Up}};
LCDDisplay lcd(NORMALPRIO);

int main (void)
{
  constexpr uint32_t MAGIC = 0xDEADBEEF;
  uint32_t magic;
  
#ifndef NOSHELL
  consoleInit();	// initialisation des objets liés au shell
#endif

  releaseResetAfter(TIME_S2I(1U)); // keep reset out value during 1 second
  FRAM::init();
  if (FRAM::read(magic, 0) == false) {
    DebugTrace("I²C Failed");
    (void) f1.setFreq(1U);
    (void) f2.setFreq(1U);
  } else if (magic == MAGIC) {
    DebugTrace("fram initialized");
    FRAM::read(frequencies[0].freq, 4);
    FRAM::read(frequencies[1].freq, 8);
    (void) f1.setFreq(frequencies[0].freq);
    (void) f2.setFreq(frequencies[1].freq);
  } else {
    DebugTrace("first run : fram NOT initialized");
    FRAM::write(MAGIC, 0);
  }
  
  Event::init(&eventCb);
  ICU::init();
  ADC adc(NORMALPRIO);
  RotaryButton rb1(NORMALPRIO, ENCODER_F1);
  RotaryButton rb2(NORMALPRIO, ENCODER_F2);
  PushButton pb1(NORMALPRIO, LINE_BOUTON_F1_SW);
  PushButton pb2(NORMALPRIO, LINE_BOUTON_F2_SW);
  BeepIn beepIn(NORMALPRIO);
  
  
  chThdCreateStatic(waBlinker, sizeof(waBlinker),
		    NORMALPRIO+2, &blinker, NULL);

#ifndef NOSHELL
  consoleLaunch();  // lancement du shell
#endif
  
  adc.run(TIME_MS2I(100));
  beepIn.run(TIME_MS2I(1));
  lcd.run(TIME_MS2I(100));
  rb1.run(TIME_MS2I(100));
  rb2.run(TIME_MS2I(100));
  pb1.run(TIME_IMMEDIATE);
  pb2.run(TIME_IMMEDIATE);
  eventCb ({Events::Turn, 0, 0});
  eventCb ({Events::Turn, 1, 0});
  lcd.enableCursor(false);

  chThdSleep(TIME_INFINITE);
}


static void eventCb(const Event& ev)
{
  auto & [lastFreq, freq, cg, dir] = frequencies[ev.getIndex()];
  int32_t inc=0;

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
    case 2 :  inc = sign; break;
    case 3 :  inc = 3*sign; break;
    default : inc  = sign * powf((deltabs-2)*2, 1.0f+(deltabs/6.0f)); break;
    }

    const uint32_t minFreqInRange = powf(10.0f, floorf(log10f(freq))) -1.0f;
    inc = std::clamp(inc, -200L, 200L);
    freq += inc * powf(10, mulExp);
    freq = std::clamp(freq, minFreqInRange, 999_khz);
    freq -= freq % static_cast<uint32_t>(powf(10, mulExp));

    dir = freq >= lastFreq ? Direction::Up : Direction::Down;

    
    break;
  }
    
  case Events::ShortClick : {
    //    DebugTrace("lastFreq=%lu freq=%lu mulExp=%lu dir=%s", lastFreq, freq, mulExp,
    //	       dir == Direction::Up ? "UP" : "DOWN");
    lastFreq = freq;
    if (dir == Direction::Up) {
      if (mulExp == 3) {
	freq = 1_hz;
      } else {
	freq = powf(10, ceilf(log10f(freq+1)));
      }
    } else { // dir == Direction::Down
      if (freq == 1)
	freq = 100_khz;
      else
	freq = powf(10, floorf(log10f(freq-1)));
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

  freq = cg.setFreq(std::clamp(freq, 1_hz, 980_khz));
  
  if (freq > 200_khz) {
    uint32_t mul = 1U;
    if (dir == Direction::Up)
      while ((freq < 980_khz) and (freq <= lastFreq)) {
   	freq = cg.setFreq(std::clamp(freq + (1_khz*mul++), 1_hz, 980_khz));
	//	DebugTrace("Iter freq UP = %lu", freq);
      }
    else
      while ((freq > 1_hz) and (freq >= lastFreq)) {
	freq = cg.setFreq(std::clamp(freq - (1_khz*mul++), 1_hz, 980_khz));
	//	DebugTrace("Iter freq DOWN = %lu", freq);
      }
  }
  FRAM::write(freq, 4+(ev.getIndex()*4));
  
  //  DebugTrace("mulExp=%ld", mulExp);
  lcd.write(1-ev.getIndex(), 5, "F%c = %s %c  ", ev.getIndex() + '1',
	    LCDDisplay::freq2Str(freq).c_str(), char(dir));
  lcd.draw();
}

