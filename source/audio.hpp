#pragma once

#include <array>
#include <algorithm>
#include <cstdint>
#include <etl/string_view.h>

using custom_dac_sample_t = uint16_t;
using decoded_pcm_sample_t = int16_t;

namespace {
  constexpr uint32_t DAC_MAX_FREQUENCY = 1000000U; // hardware limitation
  constexpr uint32_t AUDIO_SAMPLE_FREQUENCY = 32000U;
}

struct AudioLoopMetadata {
  size_t skip_samples;
  size_t play_samples;
};

namespace {

#ifndef NO_AUDIO_SET
  constexpr uint8_t psfail_mp3[] = {
#embed "../SOUNDS/MP3/psfail.mp3"
  };
  constexpr size_t psfail_mp3_len = sizeof(psfail_mp3);
#include "../SOUNDS/MP3/psfail.meta.hpp"

  constexpr uint8_t shortcut_mp3[] = {
#embed "../SOUNDS/MP3/shortcut.mp3"
  };
  constexpr size_t shortcut_mp3_len = sizeof(shortcut_mp3);
#include "../SOUNDS/MP3/shortcut.meta.hpp"

  constexpr uint8_t overvoltage_mp3[] = {
#embed "../SOUNDS/MP3/overvoltage.mp3"
  };
  constexpr size_t overvoltage_mp3_len = sizeof(overvoltage_mp3);
#include "../SOUNDS/MP3/overvoltage.meta.hpp"

#ifndef SMALL_AUDIO_SET
  constexpr uint8_t school_rings_mp3[] = {
#embed "../SOUNDS/MP3/school-rings.mp3"
  };
  constexpr size_t school_rings_mp3_len = sizeof(school_rings_mp3);
#include "../SOUNDS/MP3/school-rings.meta.hpp"

  constexpr uint8_t trumpet_mp3[] = {
#embed "../SOUNDS/MP3/trumpet.mp3"
  };
  constexpr size_t trumpet_mp3_len = sizeof(trumpet_mp3);
#include "../SOUNDS/MP3/trumpet.meta.hpp"

  constexpr uint8_t cosmos_mp3[] = {
#embed "../SOUNDS/MP3/cosmos.mp3"
  };
  constexpr size_t cosmos_mp3_len = sizeof(cosmos_mp3);
#include "../SOUNDS/MP3/cosmos.meta.hpp"

  constexpr uint8_t double_tamponne_mp3[] = {
#embed "../SOUNDS/MP3/double_tamponne.mp3"
  };
  constexpr size_t double_tamponne_mp3_len = sizeof(double_tamponne_mp3);
#include "../SOUNDS/MP3/double_tamponne.meta.hpp"
#endif
#endif
}

enum class GeneratorType {
  None,
  Sine500,
  Sine500Mod,
  SineSweep,
};

#define GENLOOPMP3N(nme, file, meta_) AudioLoop {.name = nme, \
                               .samples = file, \
                               .len = file##_len, \
                               .meta = meta_, \
                               .generator = GeneratorType::None}

#define GENLOOPPROC(nme, gen) AudioLoop {.name = nme, \
                               .samples = nullptr, \
                               .len = 0, \
                               .meta = {0, 0}, \
                               .generator = gen}

struct AudioLoop {
  const etl::string_view name;
  const uint8_t *samples;
  const size_t len;
  const AudioLoopMetadata meta;
  const GeneratorType generator;
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
  size_t getCurrentLoop(void) {return loop;}
  size_t getLoopsNumber(void) {return loops.size();}
  etl::string_view getName(const size_t index = loops.size());

private:
  void stopDac(void);
  void startDac(void);
  void startTimer(void);
  void stopTimer(void);
  bool isPlaying(void) const {return GPTD6.state == GPT_CONTINUOUS;}
  static void end_cb1(DACDriver *dacp);
  static void fillDacBuffer(custom_dac_sample_t *dest, size_t n);

  static size_t loop;
  static float attenuation;
  static custom_dac_sample_t dmaBuff[1024];
  static constexpr size_t halfBufferSize = (sizeof(dmaBuff) / sizeof(dmaBuff[0])) / 2U;

  static constexpr std::array loops = {
#ifndef NO_AUDIO_SET
				       GENLOOPPROC("sine", GeneratorType::Sine500),
				       GENLOOPPROC("sinemod", GeneratorType::Sine500Mod),
#ifndef SMALL_AUDIO_SET
				       GENLOOPPROC("sweep", GeneratorType::SineSweep),
				       GENLOOPMP3N("school", school_rings_mp3, school_rings_meta),
				       GENLOOPMP3N("trumpet", trumpet_mp3, trumpet_meta),
				       GENLOOPMP3N("cosmos", cosmos_mp3, cosmos_meta),
				       GENLOOPMP3N("dbuf", double_tamponne_mp3, double_tamponne_meta),
#endif
				       GENLOOPMP3N("psfail", psfail_mp3, psfail_meta),
				       GENLOOPMP3N("shortcut", shortcut_mp3, shortcut_meta),
				       GENLOOPMP3N("overvoltage", overvoltage_mp3, overvoltage_meta),
#else
				       AudioLoop {.name = "none", .samples = nullptr, .len = 0, .meta = {.skip_samples = 0, .play_samples = 0}, .generator = GeneratorType::None}
#endif
  };
  static const DACConfig dac1cfg1;
  static const DACConversionGroup dacgrpcfg1;
  static const GPTConfig gpt6cfg1;
};
