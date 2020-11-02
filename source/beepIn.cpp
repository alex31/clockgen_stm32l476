#include "beepIn.hpp"
#include "hardwareConf.hpp"
#include "commonRessource.hpp"
//#include "stdutil.h"


bool BeepIn::init()
{
  audio.start();
  audio.pause();
  audio.setDbVolume(storage.getVolume());
  audio.select(storage.getSampleIndex());
  return true;
}



bool BeepIn::loop()
{
  bool on = palReadLine(LINE_BUZZER_IN) == PAL_HIGH;

  if (on != lastState) {
    if (on) { 
      audio.play();
    } else {
      audio.pause();
    }
    lastState = on;
  }
  
  return true;
}



Audio BeepIn::audio;
bool  BeepIn::lastState = false;
