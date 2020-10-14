#include "beepIn.hpp"
#include "hardwareConf.hpp"
#include "audio.hpp"
//#include "stdutil.h"


namespace {
  Audio audio;
  bool  lastState = false;
}

bool BeepIn::init()
{
  audio.start();
  audio.pause();
  audio.setAttenuation(0.1f);
  audio.select(1);

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



