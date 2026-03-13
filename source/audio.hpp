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
  constexpr uint32_t LEGACY_RAW_SAMPLE_FREQUENCY = 16000U;
  constexpr size_t MP3_ENCODER_DELAY_SAMPLES = 1105U;

  // Logical loop lengths are derived from the old 16 kHz PCM assets so that
  // the MP3 path reproduces the same gapless loop duration without replaying
  // encoder delay or end padding. When regenerating the sound library, these
  // values must track the new source assets.
  constexpr size_t mp3LoopSamplesFromLegacyRaw(const size_t rawBytes)
  {
    return rawBytes * (AUDIO_SAMPLE_FREQUENCY / LEGACY_RAW_SAMPLE_FREQUENCY);
  }

#ifndef NO_AUDIO_SET
  constexpr uint8_t tone500_mp3[] = {
#embed "../SOUNDS/MP3/tone500.mp3"
  };
  constexpr size_t tone500_mp3_len = sizeof(tone500_mp3);
  constexpr size_t tone500_loop_samples = mp3LoopSamplesFromLegacyRaw(16000U);

  constexpr uint8_t tone500t10_mp3[] = {
#embed "../SOUNDS/MP3/tone500t10.mp3"
  };
  constexpr size_t tone500t10_mp3_len = sizeof(tone500t10_mp3);
  constexpr size_t tone500t10_loop_samples = mp3LoopSamplesFromLegacyRaw(16000U);

  constexpr uint8_t psfail_mp3[] = {
#embed "../SOUNDS/MP3/psfail.mp3"
  };
  constexpr size_t psfail_mp3_len = sizeof(psfail_mp3);
  constexpr size_t psfail_loop_samples = mp3LoopSamplesFromLegacyRaw(25078U);

  constexpr uint8_t shortcut_mp3[] = {
#embed "../SOUNDS/MP3/shortcut.mp3"
  };
  constexpr size_t shortcut_mp3_len = sizeof(shortcut_mp3);
  constexpr size_t shortcut_loop_samples = mp3LoopSamplesFromLegacyRaw(45976U);

  constexpr uint8_t overvoltage_mp3[] = {
#embed "../SOUNDS/MP3/overvoltage.mp3"
  };
  constexpr size_t overvoltage_mp3_len = sizeof(overvoltage_mp3);
  constexpr size_t overvoltage_loop_samples = mp3LoopSamplesFromLegacyRaw(56842U);

#ifndef SMALL_AUDIO_SET
  constexpr uint8_t school_rings_mp3[] = {
#embed "../SOUNDS/MP3/school-rings.mp3"
  };
  constexpr size_t school_rings_mp3_len = sizeof(school_rings_mp3);
  constexpr size_t school_rings_loop_samples = mp3LoopSamplesFromLegacyRaw(82217U);

  constexpr uint8_t trumpet_mp3[] = {
#embed "../SOUNDS/MP3/trumpet.mp3"
  };
  constexpr size_t trumpet_mp3_len = sizeof(trumpet_mp3);
  constexpr size_t trumpet_loop_samples = mp3LoopSamplesFromLegacyRaw(31679U);

  constexpr uint8_t sweep_mp3[] = {
#embed "../SOUNDS/MP3/sweep.mp3"
  };
  constexpr size_t sweep_mp3_len = sizeof(sweep_mp3);
  constexpr size_t sweep_loop_samples = mp3LoopSamplesFromLegacyRaw(16000U);

  constexpr uint8_t cosmos_mp3[] = {
#embed "../SOUNDS/MP3/cosmos.mp3"
  };
  constexpr size_t cosmos_mp3_len = sizeof(cosmos_mp3);
  constexpr size_t cosmos_loop_samples = mp3LoopSamplesFromLegacyRaw(537395U);
#endif
#endif
}

#define GENLOOPMP3N(nme, file, loopSamples_) AudioLoop {.name = nme, \
                               .samples = file, \
                               .len = file##_len, \
                               .skip_samples = MP3_ENCODER_DELAY_SAMPLES, \
                               .loop_samples = loopSamples_}

struct AudioLoop {
  const etl::string_view name;
  const uint8_t *samples;
  const size_t len;
  const size_t skip_samples;
  const size_t loop_samples;
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
				       GENLOOPMP3N("sine", tone500_mp3, tone500_loop_samples),
				       GENLOOPMP3N("sinemod", tone500t10_mp3, tone500t10_loop_samples),
#ifndef SMALL_AUDIO_SET
				       GENLOOPMP3N("school", school_rings_mp3, school_rings_loop_samples),
				       GENLOOPMP3N("trumpet", trumpet_mp3, trumpet_loop_samples),
				       GENLOOPMP3N("sweep", sweep_mp3, sweep_loop_samples),
				       GENLOOPMP3N("cosmos", cosmos_mp3, cosmos_loop_samples),
#endif
				       GENLOOPMP3N("psfail", psfail_mp3, psfail_loop_samples),
				       GENLOOPMP3N("shortcut", shortcut_mp3, shortcut_loop_samples),
				       GENLOOPMP3N("overvoltage", overvoltage_mp3, overvoltage_loop_samples),
#else
				       AudioLoop {.name = "none", .samples = nullptr, .len = 0, .skip_samples = 0, .loop_samples = 0}
#endif
  };
  static const DACConfig dac1cfg1;
  static const DACConversionGroup dacgrpcfg1;
  static const GPTConfig gpt6cfg1;
};
