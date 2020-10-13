#pragma once

#include <array>
#include <string_view>
#include <algorithm>

using dac8sample_t = uint8_t;


#define GENLOOPN(nme,file) AudioLoop {.name = nme, \
                              .samples = file##_raw, \
  			      .len =  file##_raw_len}
#define GENLOOP(file) AudioLoop {.name = #file,     \
                              .samples = file##_raw, \
  			      .len =  file##_raw_len}


namespace {
#include "SOUNDS/C/tone500.c"
#include "SOUNDS/C/tone500t10.c"
#include "SOUNDS/C/uk-phone.c"
#ifndef SMALL_AUDIO_SET
#include "SOUNDS/C/enzotic_alarm.c"
#include "SOUNDS/C/ahooga.c"
#include "SOUNDS/C/horn.c"
#include "SOUNDS/C/siren.c"
#include "SOUNDS/C/trumpet.c"
#include "SOUNDS/C/trumpet2.c"
#include "SOUNDS/C/police-siren.c"
#include "SOUNDS/C/nuclear-alarm.c"
#include "SOUNDS/C/sweep.c"
#include "SOUNDS/C/school-rings.c"
#endif

  constexpr uint32_t DAC_MAX_FREQUENCY = 1e6; // hardware limitation
  constexpr uint32_t SAMPLE_FREQUENCY = 8000; // expect 8khz unsigned 8 bit, one channel raw audio file
  constexpr uint32_t GPT_COUNT = DAC_MAX_FREQUENCY / SAMPLE_FREQUENCY;
}


struct AudioLoop {
  const std::string_view name;
  const dac8sample_t *samples;
  const size_t len;
};


class Audio {
public:
  void start(void);
  void select(const size_t loopIndex);
  void play(void);
  void pause(void);
  void setAttenuation(const float attn) {attenuation = std::clamp(attn, 0.0f, 1.0f);}
  size_t  getCurrentLoop(void) {return loop;}
  size_t  getLoopsNumber(void) {return loops.size();}
  std::string_view getName(const size_t index = loops.size());
  
private:
  void stopDac(void);
  void startDac(void);
  void startTimer(void);
  void stopTimer(void);
  static void end_cb1(DACDriver *dacp);
  static void audioCpy(uint8_t *dest, const uint8_t *src, size_t n);
  
  static size_t loop;
  static size_t loopLen;
  static float  attenuation;
  static const dac8sample_t *loopPtr;
  static const dac8sample_t dmaBuff[1024];
  static constexpr size_t halfBufferSize = sizeof(dmaBuff) / 2U;
  
  static constexpr std::array loops = {
				       GENLOOPN("sine", tone500),
				       GENLOOPN("sinemod", tone500t10),
				       GENLOOP(uk_phone),
#ifndef SMALL_AUDIO_SET
				       GENLOOPN("enzotic", enzotic_alarm),
				       GENLOOP(horn),
				       GENLOOP(ahooga),
				       GENLOOP(siren),
				       GENLOOPN("school", school_rings),
				       GENLOOP(trumpet),
				       GENLOOP(trumpet2),
				       GENLOOPN("police", police_siren),
				       GENLOOPN("nuclear", nuclear_alarm),
				       GENLOOP(sweep),
#endif
  };
  static const DACConfig dac1cfg1;
  static const DACConversionGroup dacgrpcfg1;
  static const GPTConfig gpt6cfg1;
};
