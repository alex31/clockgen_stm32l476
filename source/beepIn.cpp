#include "beepIn.hpp"
#include "hardwareConf.hpp"
#include "storage.hpp"
//#include "stdutil.h"


bool BeepIn::init()
{
  audio.start();
  audio.pause();
  audio.setDbVolume(Storage::instance().getVolume());
  audio.select(Storage::instance().getSampleIndex());
  return true;
}



bool BeepIn::loop()
{
  bool on = palReadLine(LINE_BUZZER_IN) == PAL_HIGH;

  if (on != lastState) {
    lastChangeTs = chVTGetSystemTime();
    if (on) { 
      audio.play();
    } else {
      audio.pause();
    }
    lastState = on;
  }
  
  return true;
}

bool BeepIn::hasRecentChange(void)
{
  return TIME_I2S(chTimeDiffX(lastChangeTs, chVTGetSystemTime())) < 2;
}

Audio BeepIn::audio;
bool  BeepIn::lastState = false;
systime_t BeepIn::lastChangeTs=0;
