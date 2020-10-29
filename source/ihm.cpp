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
#include "commonRessource.hpp"
#include "stdutil.h"

namespace {
  LCDDisplay lcd(NORMALPRIO);
  void eventCb(const Event& ev) {
    LcdTab::propagate(ev); 
  }
  constexpr size_t lineOfDisplaySystem = 9U;
  constexpr size_t lineOfDisplayStatus = 8U;
  void displaySystemCb(FrameBuffer<LCD_WIDTH, lineOfDisplaySystem> &fb);
  void displayStatusCb(FrameBuffer<LCD_WIDTH, lineOfDisplayStatus> &fb);
  
  // order of declaration matters
  RotaryButton rb2(NORMALPRIO, ENCODER_F2);
  RotaryButton rb1(NORMALPRIO, ENCODER_F1);
  PushButton pb2(NORMALPRIO, LINE_BOUTON_F2_SW);
  PushButton pb1(NORMALPRIO, LINE_BOUTON_F1_SW);

  MenuEntries<10, 16> audioSample{"sample", 10, 0, {}}; // build dynamically
  NumericEntry<10> audioVol{"volume", 10, 0, 10, 1, {0, 25}};

  MenuEntries<10, 16> info{"info", 10, 0, {
					   {1, "manual", StateId::Manual}, 
					   {2, "system", StateId::System},
					   {3, "status", StateId::Status}
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

  MenuEntries<20, 4> logicVoltage{"logic Vcc", 0, 0,
				   {{33, "Logic 3.3V [Func]"},
				    {50, "Logic 5.0V [74xx]"}
				   }};

  ScrollText stManual("manual",
		      FrameBuffer<LCD_WIDTH, 36U>
		      {"-------MANUAL-------",
		       "*tourner pour augm- ",
		       "-enter ou baisser la",
		       "frequence pour F1 en",
		       "haut et F2 en bas.  ",
		       "* Un appui court    ",
		       "permet de parcourir ",
		       "rapidement la gamme ",
		       "dans le sens de la  ",
		       "fleche.             ",
		       "* Un appui          ",
		       "long donne acces au ",
		       "menu des raccourcis ",
		       "pour les frequences ",
		       "usuelles.           ",
		       "* Un double clic sur",
		       "le bouton du haut   ",
		       "permet de regler le ",
		       "volume, et le type  ",
		       "de son, et d'avoir  ",
		       "acces au pages      ",
		       "d'information syste-",
		       "me.                 ",
		       "* Un appui sur le   ",
		       "bouton du haut a la ",
		       "mise sous tension   ",
		       "permet de selectio- ",
		       "nner la tension     ",
		       "attendue de la      ",
		       "logique (3V ou 5V)  ",
		       "* Une sous tension  ",
		       "ou une surtension   ",
		       "sur la tension logi-",
		       "que declenche une   ",
		       "alarme sonore.      ",
			 }
		      );

						  
  ScrollText<lineOfDisplaySystem> stSystem("system", &displaySystemCb);
  ScrollText<lineOfDisplayStatus> stStatus("status", &displayStatusCb);
  

  MainTab mainTab(StateId::Freq);
  OneColTab freqShortCut(StateId::FreqShortCut, &frequencies);
  OneColTab voltageChoice(StateId::VoltageChoice, &logicVoltage);
  TwoColsTab param(StateId::Param, {&audioSample, &audioVol, &info});
  OneColTab manual(StateId::Manual, &stManual);
  OneColTab system(StateId::System, &stSystem);
  OneColTab status(StateId::Status, &stStatus);
}

void IHM::init()
{
  Event::init(&eventCb);
  lcd.run(TIME_MS2I(100));
  lcd.enableCursor(false);

  Audio& audio = BeepIn::getAudio();

  for (size_t i=0; i< audio.getLoopsNumber(); i++) {
    audioSample.addEntry(Entry{int(i),
			       FixedStr(audio.getName(i).data())});
  }

  audioVol.bind([&audio] (uint32_t val) {
		  const uint32_t attn = 25 - val;
		  storage.setVolume(attn);

		  const float logAttn = powf(10, attn / 10.0f);
		  audio.setAttenuation(1.0f/logAttn);
		  audio.play();
		});

  audioSample.bind([&audio] (uint32_t val) {
		  storage.setSampleIndex(val);
		  audio.select(val);
		  audio.play();
		});
  param.bind(LcdTab::Leave, [&audio]{
 			   audio.pause();
 			 });
   FrameBufferBase::setPrintFn([](uint8_t posx, 
 				 uint8_t posy,
 				 const char* str) {
 				lcd.write(posy, posx, str);
 			      });
   
   frequencies.bind([] (uint32_t val) {
		     mainTab.setFreq(val);
		   });

   logicVoltage.bind([] (uint32_t val) {
		     storage.setVoltageRef(val/10.0f);
		   });

   // initial choice when entering in menu must reflex reality
   voltageChoice.bind(LcdTab::Enter, [] {
				       if (adc.getLogicVoltage() < 4.0f) 
					 logicVoltage.set(0);
				       else 
					 logicVoltage.set(1);
				     });
  LcdTab::push(StateId::Freq);
  rb1.run(TIME_MS2I(100));
  rb2.run(TIME_MS2I(100));
  pb1.run(TIME_IMMEDIATE);
  pb2.run(TIME_IMMEDIATE);
}

#define XSTR(s) STR(s)
#define STR(s) #s

namespace {
  void displaySystemCb(FrameBuffer<LCD_WIDTH, lineOfDisplaySystem> &fb)
  {
    size_t i=0;
    fb.write(0, i++, "Branch=%s", XSTR(GIT_BRANCH));
    fb.write(0, i++, "Tag=%s", XSTR(GIT_TAG));
    fb.write(0, i++, "Vers=%s", XSTR(GIT_SHA));
    fb.write(0, i++, "Kernel %s", CH_KERNEL_VERSION);
    fb.write(0, i++, "Hal  %s", HAL_VERSION);
    fb.write(0, i++, "%s", PORT_COMPILER_NAME);
    fb.write(0, i++, "Build Time%c",':');
    fb.write(0, i++, "%s", __DATE__);
    fb.write(0, i++, "%s", __TIME__);
  }


  void displayStatusCb(FrameBuffer<LCD_WIDTH, lineOfDisplayStatus> &fb)
  {
    size_t i=0;
    uint32_t min = storage.getAge() / 60;
    uint32_t hour = min / 60;
    const uint32_t day = hour / 24;
    min %= 60;
    hour %= 24;

    uint32_t s = TIME_I2S(chVTGetSystemTimeX());
    uint32_t m = s / 60;
    uint32_t h = m / 60;
    s %= 60;
    m %= 60;
    
    fb.write(0, i++, "Vpower= %.2f", adc.getPowerSupplyVoltage());
    fb.write(0, i++, "Vlogic= %.2f", adc.getLogicVoltage());
    fb.write(0, i++, "Age= %dd, %dh, %dm", day, hour, min);
    fb.write(0, i++, "Cycles= %d", storage.getPowerOn());
    fb.write(0, i++, "Logic Voltage = %.2f", storage.getVoltageRef());
    fb.write(0, i++, "OverVoltage= %d", storage.getOverVoltageAlert());
    fb.write(0, i++, "UnderVoltage= %d", storage.getUnderVoltageAlert());
    fb.write(0, i++, "Systime= %d:%d:%d", h, m, s);
  }
}

