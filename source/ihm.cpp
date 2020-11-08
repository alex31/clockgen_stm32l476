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
#include "adc.hpp"
#include "stdutil.h"

namespace {
  LCDDisplay lcd(NORMALPRIO-2);
  void eventCb(const Event& ev) {
    LcdTab::propagate(ev); 
  }
  constexpr size_t lineOfDisplaySystem = 9U;
  constexpr size_t lineOfDisplayStatus = 10U;
  constexpr size_t lineOfDisplayAdcAlert = 4U;
  void displaySystemCb(FrameBuffer<LCD_WIDTH, lineOfDisplaySystem> &fb);
  void displayStatusCb(FrameBuffer<LCD_WIDTH, lineOfDisplayStatus> &fb);
  void displayAdcAlertCb(FrameBuffer<LCD_WIDTH, lineOfDisplayAdcAlert> &fb);

  Event psHealthTrigged;
  // order of declaration matters
  RotaryButton rb2(NORMALPRIO, ENCODER_F2);
  RotaryButton rb1(NORMALPRIO, ENCODER_F1);
  PushButton pb2(NORMALPRIO, LINE_BOUTON_F2_SW);
  PushButton pb1(NORMALPRIO, LINE_BOUTON_F1_SW);

  MenuEntries<10, 16> audioSample{"sample", 10, 0, {}}; // build dynamically
  NumericEntry<10> audioVol{"volume", 10, 0, 10, 5, {0, 100}};

  MenuEntries<10, 4> info{"info", 10, 0, {
					   {1, "manual", StateId::Manual}, 
					   {2, "system", StateId::System},
					   {3, "status", StateId::Status}
					   }};

  MenuEntries<20, 16> frequencies{"freq.", 0, 0, {
					 {1_hz, "1 Hz"},
					 {10_hz, "10 Hz"},
					 {100_hz, "100 Hz"},
					 {1_khz, "1 Khz"},
					 {2.4_khz, "2.4 Khz"},
					 {4.8_khz, "4.8 Khz"},
					 {8_khz, "8 Khz"},
					 {9600, "9.6 Khz"},
					 {10_khz, "10 Khz"},
					 {100_khz, "100 Khz"}
				 }};

  MenuEntries<20, 4> logicVoltage{"logic Vcc", 0, 0,
				   {{33, "Logic 3.3V [Func]"},
				    {50, "Logic 5.0V [74xx]"}
				   }};

  MenuEntries<20, 4> clearDefault{"clear D", 0, 0,
				   {{0, "Keep defaults"},
				    {1, "Clear defaults"}
				   }};
  
 ScrollText stManual("manual",
		      FrameBuffer<LCD_WIDTH, 50U>
		      {"-MANUEL UTILISATEUR-",
		       "                    ",
		       "*tourner pour augm- ",
		       "-enter ou baisser la",
		       "frequence pour F1 en",
		       "haut et F2 en bas.  ",
		       "                    ",
		       "* Un appui court    ",
		       "permet de parcourir ",
		       "rapidement la gamme ",
		       "dans le sens de la  ",
		       "fleche.             ",
		       "                    ",
		       "* Un appui          ",
		       "long donne acces au ",
		       "menu des raccourcis ",
		       "pour les frequences ",
		       "usuelles.           ",
		       "                    ",
		       "* Un signal branche ",
		       "sur l'entree frequ- ",
		       "encemetre declenche-",
		       "ra l'affichage de la",
		       "frequence, du temps ",
		       "de l'impulsion posi-",
		       "tive et du rapport  ",
		       "cyclique.           ",
		       "                    ",
		       "* Un double clic sur",
		       "le bouton du haut   ",
		       "permet de regler le ",
		       "volume, et le type  ",
		       "de son, et d'avoir  ",
		       "acces au pages      ",
		       "d'information syste-",
		       "me et de status.    ",
		       "                    ",
		       "* Un appui sur le   ",
		       "bouton du haut a la ",
		       "mise sous tension   ",
		       "permet de selectio- ",
		       "nner la tension     ",
		       "attendue de la      ",
		       "logique (3V ou 5V)  ",
		       "                    ",
		       "* Une sous tension  ",
		       "ou une surtension   ",
		       "sur la tension logi-",
		       "que declenche une   ",
		       "alarme sonore.      ",
			 }
		      );

						  
  ScrollText<lineOfDisplaySystem> stSystem("system", &displaySystemCb);
  ScrollText<lineOfDisplayStatus> stStatus("status", &displayStatusCb);
  ScrollText<lineOfDisplayAdcAlert> stAdcAlert("adcAlrt", &displayAdcAlertCb);

  

  MainTab mainTab(StateId::Freq);
  OneColTab freqShortCut(StateId::FreqShortCut, &frequencies);
  OneColTab voltageChoice(StateId::VoltageChoice, &logicVoltage);
  OneColTab defaultsClear(StateId::ClearDefault, &clearDefault);
  TwoColsTab param(StateId::Param, {&info, &audioVol, &audioSample});
  OneColTab manual(StateId::Manual, &stManual);
  OneColTab system(StateId::System, &stSystem);
  OneColTab status(StateId::Status, &stStatus, StateId::ClearDefault);
  OneColTab adcAlert(StateId::VoltageAlert, &stAdcAlert);
}

void IHM::init()
{
  Event::init(&eventCb);
  lcd.run(TIME_MS2I(100));
  lcd.enableCursor(false);

  Audio& audio = BeepIn::getAudio();

  for (size_t i=0; i< audio.getLoopsNumber(); i++) {
    if (const auto& n = audio.getName(i);
	(n.compare("psfail") != 0) && 	(n.compare("shortcut") != 0) &&
	(n.compare("overvoltage") != 0)) {
      audioSample.addEntry(Entry{int(i),
				 FixedStr(audio.getName(i))});
    }
  }
  audioSample.set(storage.getSampleIndex());
  audioVol.set(storage.getVolume());

  audioVol.bind([&audio] (uint32_t val) {
		  storage.setVolume(val);
		  audio.setDbVolume(val);
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
   voltageChoice.bind(LcdTab::Enter, [&audio] {
				       if (ADC::getLogicVoltage() < 4.0f) 
					 logicVoltage.set(0);
				       else 
					 logicVoltage.set(1);
				     });
   defaultsClear.bind(LcdTab::Leave, [] {
				       if (defaultsClear.get() != 0) {
					 storage.resetAlert();
				       }
				     });
   adcAlert.bind(LcdTab::Enter, [&audio] {
				  psHealthTrigged = ADC::getVoltageHealth();
				  // psHealthTrigged.print("lcdtab enter");
				  audio.pause();
				  if (psHealthTrigged.getIndex() == ADC::PowerSupply) {
				    storage.incPsFailureAlert();
				    audio.select("psfail");
				  } else {
				    if (psHealthTrigged.getEvent() == Events::UnderVoltage) {
				      storage.incUnderVoltageAlert();
				      audio.select("shortcut");
				    } else {
				      storage.incOverVoltageAlert();
				      audio.select("overvoltage");
				    }
				  }
				  audio.setAttenuation(0.70f);
				  audio.play();
				});
   adcAlert.bind(LcdTab::Leave, [&audio] {
				  audio.pause();
				  audio.select(storage.getSampleIndex());
				  audio.setDbVolume(storage.getVolume());
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
    
    fb.write(0, i++, "Vpower= %.2f  ", ADC::getPowerSupplyVoltage());
    fb.write(0, i++, "Vlogic= %.2f  ", ADC::getLogicVoltage());
    fb.write(0, i++, "VLogic Ref = %.2f  ", storage.getVoltageRef());
    fb.write(0, i++, "Age= %dd, %dh, %dm    ", day, hour, min);
    fb.write(0, i++, "Cycles= %d  ", storage.getPowerOn());
    fb.write(0, i++, "FRam i2c Fail = %d  ", storage.getI2cFailure());
    fb.write(0, i++, "PS Failure= %d  ", storage.getPsFailureAlert());
    fb.write(0, i++, "OverVoltage= %d  ", storage.getOverVoltageAlert());
    fb.write(0, i++, "UnderVoltage= %d  ", storage.getUnderVoltageAlert());
    fb.write(0, i++, "Systime= %.2d:%.2d:%.2d    ", h, m, s);
  }


  void displayAdcAlertCb(FrameBuffer<LCD_WIDTH, lineOfDisplayAdcAlert> &fb)
  {
    size_t i=0;
    const Event currentHealth = ADC::getVoltageHealth();
    const bool defaultActive = currentHealth.getEvent() != Events::None;
    Audio& audio = BeepIn::getAudio();
    if (defaultActive)
      audio.play();
    else
      audio.pause();
    //    psHealthTrigged.print("display alert");

    if (psHealthTrigged.getIndex() == ADC::PowerSupply) {
      fb.write(0, i++, "Pow.Supply FAILURE  ");
      fb.write(0, i++, "Status = %s%*c",
	       defaultActive ? "ACTIVE" : "clear", 10, ' ');
    } else if (psHealthTrigged.getIndex() == ADC::Logic) {
      const char *defaultType =
	psHealthTrigged.getEvent() == Events::UnderVoltage ?
	"SHORTCUT" : "OVERVOLTAGE";
      fb.write(0, i++, "Logic %s%*c", defaultType, 10, ' ');
      fb.write(0, i++, "Status = %s%*c",
	       defaultActive ? "ACTIVE" : "clear", 10, ' ');
    } else  {
      fb.write(0, i++, "New ref. registered", 10, ' ');
      fb.write(0, i++, "Double check voltage", 10, ' ');
    }
    fb.write(0, i++, "Vpower= %.2f%*c", ADC::getPowerSupplyVoltage(), 10, ' ');
    fb.write(0, i++, "Vlogic= %.2f%*c", ADC::getLogicVoltage(), 10, ' ');
  }
  
}


