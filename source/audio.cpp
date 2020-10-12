#include "audio.hpp"
#include "event.hpp"
#include "hardwareConf.hpp"
#include "stdutil.h"


namespace {
  static const DACConfig dac1cfg1 = {
				     .init         = 127U,
				     .datamode     = DAC_DHRM_8BIT_RIGHT,
				     .cr           = 0
  };

  //  dacsample_t IN_DMA_SECTION_NOINIT(dacSamples[512]);
}

bool AUDIO::init()
{
  DebugTrace("AUDIO::init()");
  dacStart(&DACD1, &dac1cfg1);
  //  dacPutChannelX(&DACD1, 0, 2047U);
  return true;
}



bool AUDIO::loop()
{
  return true;
}

