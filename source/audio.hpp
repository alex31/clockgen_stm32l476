#pragma once

#include <array>
#include <string_view>
#include <algorithm>

using custom_dac_sample_t = uint16_t;
using source_dac_sample_t = uint8_t;

#define GENLOOPN(nme,file) AudioLoop {.name = nme, \
                              .samples = file##_raw, \
  			      .len =  file##_raw_len}
#define GENLOOP(file) AudioLoop {.name = #file,     \
                              .samples = file##_raw, \
  			      .len =  file##_raw_len}


namespace {
#ifndef NO_AUDIO_SET
#include "SOUNDS/C/tone500.c"
#include "SOUNDS/C/tone500t10.c"
#include "SOUNDS/C/psfail.c"
#include "SOUNDS/C/shortcut.c"
#include "SOUNDS/C/overvoltage.c"
#ifndef SMALL_AUDIO_SET
#include "SOUNDS/C/trumpet.c"
#include "SOUNDS/C/sweep.c"
#include "SOUNDS/C/school-rings.c"
#include "SOUNDS/C/cosmos.c"
#endif
#endif
  constexpr uint32_t DAC_MAX_FREQUENCY = 1e6; // hardware limitation
  constexpr uint32_t SAMPLE_FREQUENCY = 16000; // expect 16khz unsigned 8 bit, one channel raw audio file
  constexpr uint32_t GPT_COUNT = DAC_MAX_FREQUENCY / SAMPLE_FREQUENCY;
}


struct AudioLoop {
  const std::string_view name;
  const source_dac_sample_t *samples;
  const size_t len;
};


class Audio {
public:
  void start(void);
  void select(const size_t loopIndex);
  void select(const char* sampleName);
  void play(void);
  void pause(void);
  void setAttenuation(const float attn) {attenuation = std::clamp(attn, 0.0f, 1.0f);}
  void setDbVolume(const uint8_t volume);
  size_t  getCurrentLoop(void) {return loop;}
  size_t  getLoopsNumber(void) {return loops.size();}
  std::string_view getName(const size_t index = loops.size());
  
private:
  void stopDac(void);
  void startDac(void);
  void startTimer(void);
  void stopTimer(void);
  bool isPlaying(void) const {return GPTD6.state == GPT_CONTINUOUS;}
  static void end_cb1(DACDriver *dacp);
  static void audioCpy(custom_dac_sample_t  *dest, const source_dac_sample_t *src, size_t n);
  
  static size_t loop;
  static size_t loopLen;
  static float  attenuation;
  static const source_dac_sample_t *loopPtr;
  static const custom_dac_sample_t dmaBuff[1024];
  static constexpr size_t halfBufferSize = (sizeof(dmaBuff) / sizeof(dmaBuff[0])) / 2U;
  
  static constexpr std::array loops = {
#ifndef NO_AUDIO_SET		       
				       GENLOOPN("sine", tone500),
				       GENLOOPN("sinemod", tone500t10),
#ifndef SMALL_AUDIO_SET
				       GENLOOPN("school", school_rings),
				       GENLOOP(trumpet),
				       GENLOOP(sweep),
				       GENLOOP(cosmos),
#endif
				       GENLOOP(psfail),
				       GENLOOP(shortcut),
				       GENLOOP(overvoltage),
#else
				       // just to see the binary size without
				       // sound; not meant to be flashed or run
AudioLoop {.name = "none", .samples = nullptr, .len = 0}				       
#endif
  };
  static const DACConfig dac1cfg1;
  static const DACConversionGroup dacgrpcfg1;
  static const GPTConfig gpt6cfg1;
};
