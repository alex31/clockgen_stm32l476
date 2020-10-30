#include "beepIn.hpp"
#include "hardwareConf.hpp"
#include "commonRessource.hpp"
//#include "stdutil.h"


bool BeepIn::init()
{
  audio.start();
  audio.pause();
  const float logAttn = powf(10, storage.getVolume() / 10.0f);
  audio.setAttenuation(1.0f / logAttn);
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
