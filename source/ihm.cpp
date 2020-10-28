#include "ihm.hpp"
#include "clockGenerator.hpp"
#include "rotaryButton.hpp"
#include "pushButton.hpp"
#include "hardwareConf.hpp"
#include "menuEntries.hpp"
#include "mainTab.hpp"
#include "twoColsTab.hpp"
#include "oneColTab.hpp"
#include "commonRessource.hpp"
#include "beepIn.hpp"
#include "stdutil.h"

namespace {
  LCDDisplay lcd(NORMALPRIO);
  static void eventCb(const Event& ev) {
    LcdTab::propagate(ev); 
  }

  // order of declaration matters
  RotaryButton rb2(NORMALPRIO, ENCODER_F2);
  RotaryButton rb1(NORMALPRIO, ENCODER_F1);
  PushButton pb2(NORMALPRIO, LINE_BOUTON_F2_SW);
  PushButton pb1(NORMALPRIO, LINE_BOUTON_F1_SW);

  MenuEntries<10, 16> audioSample{"sample", 10, 0, {}}; // build dynamically
  NumericEntry<10> audioVol{"volume", 10, 0, 10, 1, {0, 100}};

  MenuEntries<10, 16> info{"info", 10, 0, {
					   {1, "manuel"},
				   {2, "system"},
				   {3, "events"}
					       }};

  MenuEntries<20, 16> frequencies{"freq.", 0, 0, {{1, "1_Hz"},
					 {20, "20_Hz"},
					 {50, "50_Hz"},
					 {300, "300_hz"},
					 {1000, "1_Khz"},
					 {2400, "2.4_Khz"},
					 {8000, "8_Khz"},
					 {9600, "9.6_Khz"},
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
}


void IHM::init()
{
  Event::init(&eventCb);
  lcd.run(TIME_MS2I(100));
  rb1.run(TIME_MS2I(100));
  rb2.run(TIME_MS2I(100));
  pb1.run(TIME_IMMEDIATE);
  pb2.run(TIME_IMMEDIATE);
  lcd.enableCursor(false);

  Audio& audio = BeepIn::getAudio();

  for (size_t i=0; i< audio.getLoopsNumber(); i++) {
    audioSample.addEntry(Entry{int(i),
			       FixedStr(audio.getName(i).data())});
  }

  audioVol.bind([&audio] (uint32_t val) {
		  storage.setVolume(val);
		  const float att = val / 100.0f;
		  audio.setAttenuation(att);
		  audio.play();
		});

  audioSample.bind([&audio] (uint32_t val) {
		  storage.setSampleIndex(val);
		  audio.select(val);
		  audio.play();
		});
  tc.bind(LcdTab::Leave, [&audio]{
			   audio.pause();
			 });
  FrameBufferBase::setPrintFn([](uint8_t posx, 
				 uint8_t posy,
				 const char* str) {
				lcd.write(posy, posx, str);
			      });
  frequencies.bind([] (uint32_t val) {
		     mt.setFreq(val);
		   });
  
  LcdTab::push(StateId::Freq);
}

