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
#include "menuEntries.hpp"
#include "mainTab.hpp"
#include "twoColsTab.hpp"
#include "oneColTab.hpp"

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
  lcd.enableCursor(false);
  
  FrameBufferBase::setPrintFn([](uint8_t posx,
				 uint8_t posy,
				 const char* str) {
				lcd.writeImmediate(posy, posx, str);
			      });

  MenuEntries<10, 16> audioSample{"sample", 10, 0, {
				    {1, "hoorn"},
	                            {2, "tone"},
				    {3, "alarm"},
				    {4, "drift"},
				    {5, "siren"},
				    {6, "nuclear"},
				    {7, "fire"}
    }};

  NumericEntry<10> audioVol{"volume", 10, 0, 30, 10, {0, 100}};

  MenuEntries<10, 16> info{"info", 10, 0, {
				   {1, "manuel"},
				   {2, "readme"},
				   {3, "events"}
					       }};

  MenuEntries<20, 16> frequencies{"freq.", 0, 0, {{1, "1_Hz"},
					 {20, "20_Hz"},
					 {300, "300_hz"},
					 {400, "400_hz"},
					 {2400, "2.4_Khz"},
					 {5000, "5_Khz"},
					 {8000, "8_Khz"},
					 {10000, "10_Khz"},
					 {19200, "19.2_Khz"},
					 {36400, "36.4_Khz"}
					 }};
  ScrollText st1("readme",
		FrameBuffer<LCD_WIDTH, 12U>
		{"-------README-------",
		 "tourner pour augmen-",
		 "-er ou baisser la   ",
		 "fréquence pour      ",
		 "F1 en haut et F2 en ",
		 "bas. Un appui court ",
		 "permet de parcourir ",
		 "rapidement la gamme ",
		 "dans le sens de la  ",
		 "fleche. Un appui    ",
		 "long donne acces au ",
		 "menu des raccourcis "}
		);

  ScrollText<6U> st2("readme",
   		 [](FrameBuffer<LCD_WIDTH, 6U> &fb) {
   		   fb.write(0, 0, "mon nouveau %c", ' ');
   		   fb.write(0, 1, "contenu 1%c", ' ');
   		   fb.write(0, 2, "contenu 2%c", ' ');
   		   fb.write(0, 3, "contenu 3%c", ' ');
   		   fb.write(0, 4, "contenu 4%c", ' ');
   		   fb.write(0, 5, "contenu 5%c", ' ');
   		 });
  
  
  MainTab mt(StateId::Freq);
  OneColTab sc(StateId::FreqShortCut, &frequencies);
  TwoColsTab tc(StateId::Info, {&audioSample, &audioVol, &info});
  OneColTab rm(StateId::Readme, &st2);
  LcdTab::push(StateId::Freq);
  eventCb ({Events::Turn, 0, 0});
  eventCb ({Events::Turn, 1, 0});
  
  chThdSleep(TIME_INFINITE);
}


static void eventCb(const Event& ev)
{
 LcdTab::propagate(ev); 
}

