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
#include "beepIn.hpp"
#include "menuEntries.hpp"
#include "mainTab.hpp"
#include "twoColsTab.hpp"
#include "oneColTab.hpp"
#include "commonRessource.hpp"

static void eventCb(const Event& ev);


   
void _init_chibios() __attribute__ ((constructor(101)));
void _init_chibios() {
  halInit();
  chSysInit();
  initHeap ();
}
namespace std {
  void __throw_bad_function_call() { chSysHalt("throw_bad_function_call"); while(true);};
}

int main (void)
{
#ifndef NOSHELL
  consoleInit();	// initialisation des objets liés au shell
#endif

  releaseResetAfter(TIME_S2I(1U)); // keep reset out value during 1 second
  FRAM::init();
 
  Event::init(&eventCb);
  ICU::init();
  ADC adc(NORMALPRIO);

  // order of declaration matters
  RotaryButton rb2(NORMALPRIO, ENCODER_F2);
  RotaryButton rb1(NORMALPRIO, ENCODER_F1);
  PushButton pb2(NORMALPRIO, LINE_BOUTON_F2_SW);
  PushButton pb1(NORMALPRIO, LINE_BOUTON_F1_SW);
  BeepIn beepIn(NORMALPRIO);
  
  
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
				lcd.write(posy, posx, str);
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
  frequencies.bind([&mt] (uint32_t val) {
		     mt.setFreq(val);
		   });
  OneColTab sc(StateId::FreqShortCut, &frequencies);
  TwoColsTab tc(StateId::Info, {&audioSample, &audioVol, &info});
  OneColTab rm(StateId::Readme, &st2);
  LcdTab::push(StateId::Freq);

  chThdSleep(TIME_INFINITE);
}


static void eventCb(const Event& ev)
{
 LcdTab::propagate(ev); 
}

