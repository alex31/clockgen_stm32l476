#include <ch.h>
#include <hal.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <atomic>
#include <numbers>
#include "stdutil.h"
#include "audio.hpp"

extern "C" {
#include "../ext/helix_mp3/pub/mp3dec.h"
}

namespace {
  constexpr size_t mp3MaxOutputSamples = MAX_NGRAN * MAX_NSAMP * MAX_NCHAN;
  constexpr size_t mp3MonoFrameSamples = MAX_NGRAN * MAX_NSAMP;
  constexpr size_t mp3PcmRingSize = 4096U;
  constexpr size_t mp3PrimeSamples = 1536U;
  constexpr size_t mp3LowWatermark = 2048U;
  constexpr tprio_t mp3DecoderPriority = NORMALPRIO;

  THD_WORKING_AREA(waMp3Decoder, 4096U);

  HMP3Decoder mp3Decoder = nullptr;
  MP3FrameInfo mp3FrameInfo = {};
  decoded_pcm_sample_t mp3DecodedFrame[mp3MaxOutputSamples] = {};
  decoded_pcm_sample_t mp3MonoFrame[mp3MonoFrameSamples] = {};
  decoded_pcm_sample_t mp3PcmRing[mp3PcmRingSize] = {};
  volatile size_t mp3PcmReadIdx = 0U;
  volatile size_t mp3PcmWriteIdx = 0U;
  std::atomic<size_t> mp3PcmCount = 0U;
  const AudioLoop *activeLoop = nullptr;
  const uint8_t *mp3StreamStart = nullptr;
  const uint8_t *mp3StreamPtr = nullptr;
  size_t mp3StreamLen = 0U;
  size_t mp3BytesLeft = 0U;
  size_t mp3SkipSamples = 0U;
  size_t mp3LoopSamplesRemaining = 0U;
  bool mp3Active = false;
  bool mp3ThreadStarted = false;

  float synthPhase = 0.0f;
  float synthModPhase = 0.0f;

  size_t mp3RingFreeSpace(void)
  {
    return mp3PcmRingSize - mp3PcmCount.load(std::memory_order_relaxed);
  }

  void mp3ResetRing(void)
  {
    mp3PcmReadIdx = 0U;
    mp3PcmWriteIdx = 0U;
    mp3PcmCount.store(0U, std::memory_order_relaxed);
  }

  void mp3ResetDecoder(void)
  {
    if (mp3Decoder != nullptr) {
      MP3FreeDecoder(mp3Decoder);
    }
    mp3Decoder = MP3InitDecoder();
    std::memset(&mp3FrameInfo, 0, sizeof(mp3FrameInfo));
  }

  void mp3Rewind(void)
  {
    mp3StreamPtr = mp3StreamStart;
    mp3BytesLeft = mp3StreamLen;
    mp3ResetDecoder();
  }

  void mp3Prepare(const AudioLoop &audioLoop)
  {
    activeLoop = &audioLoop;
    mp3StreamStart = audioLoop.samples;
    mp3StreamLen = audioLoop.len;
    mp3SkipSamples = audioLoop.meta.skip_samples;
    mp3LoopSamplesRemaining = audioLoop.meta.play_samples;
    mp3Active = false;
    mp3ResetRing();
    mp3Rewind();
  }

  size_t mp3DownmixToMono(const decoded_pcm_sample_t *src,
			  const size_t outputSamps,
			  const int nChans)
  {
    if (nChans <= 1) {
      std::copy_n(src, outputSamps, mp3MonoFrame);
      return outputSamps;
    }

    const size_t frames = outputSamps / static_cast<size_t>(nChans);
    for (size_t i = 0; i < frames; i++) {
      const int32_t left = src[i * 2U + 0U];
      const int32_t right = src[i * 2U + 1U];
      mp3MonoFrame[i] = static_cast<decoded_pcm_sample_t>((left + right) / 2);
    }
    return frames;
  }

  void mp3PublishSamples(const decoded_pcm_sample_t *samples, const size_t n)
  {
    size_t writeIdx = mp3PcmWriteIdx;
    for (size_t i = 0; i < n; i++) {
      mp3PcmRing[writeIdx] = samples[i];
      writeIdx++;
      if (writeIdx == mp3PcmRingSize) {
	writeIdx = 0U;
      }
    }

    mp3PcmWriteIdx = writeIdx;
    mp3PcmCount.fetch_add(n, std::memory_order_relaxed);
  }

  bool mp3PublishLoopSamples(const decoded_pcm_sample_t *samples, size_t n)
  {
    bool published = false;

    while (n != 0U) {
      if (mp3SkipSamples != 0U) {
	const size_t skipped = std::min(n, mp3SkipSamples);
	samples += skipped;
	n -= skipped;
	mp3SkipSamples -= skipped;
	continue;
      }

      if (mp3LoopSamplesRemaining == 0U) {
	mp3SkipSamples = activeLoop->meta.skip_samples;
	mp3LoopSamplesRemaining = activeLoop->meta.play_samples;
	mp3Rewind();
	break;
      }

      const size_t chunk = std::min(n, mp3LoopSamplesRemaining);
      if (chunk > mp3RingFreeSpace()) {
	break;
      }

      mp3PublishSamples(samples, chunk);
      published = true;
      samples += chunk;
      n -= chunk;
      mp3LoopSamplesRemaining -= chunk;

      if (mp3LoopSamplesRemaining == 0U) {
	mp3SkipSamples = activeLoop->meta.skip_samples;
	mp3LoopSamplesRemaining = activeLoop->meta.play_samples;
	mp3Rewind();
	break;
      }
    }

    return published;
  }

  bool mp3DecodeOneFrame(void)
  {
    if (activeLoop == nullptr || mp3Decoder == nullptr || activeLoop->generator != GeneratorType::None) {
      return false;
    }

    if (mp3BytesLeft < 4U) {
      mp3Rewind();
    }

    int offset = MP3FindSyncWord(const_cast<unsigned char *>(mp3StreamPtr),
				 static_cast<int>(mp3BytesLeft));
    if (offset < 0) {
      mp3Rewind();
      offset = MP3FindSyncWord(const_cast<unsigned char *>(mp3StreamPtr),
			       static_cast<int>(mp3BytesLeft));
      if (offset < 0) {
	return false;
      }
    }

    mp3StreamPtr += static_cast<size_t>(offset);
    mp3BytesLeft -= static_cast<size_t>(offset);

    unsigned char *readPtr = const_cast<unsigned char *>(mp3StreamPtr);
    int bytesLeft = static_cast<int>(mp3BytesLeft);
    const int err = MP3Decode(mp3Decoder, &readPtr, &bytesLeft, mp3DecodedFrame, 0);
    mp3StreamPtr = readPtr;
    mp3BytesLeft = static_cast<size_t>(std::max(bytesLeft, 0));

    if (err != ERR_MP3_NONE) {
      if (err == ERR_MP3_INDATA_UNDERFLOW || err == ERR_MP3_FREE_BITRATE_SYNC) {
	mp3Rewind();
      } else if (mp3BytesLeft > 0U) {
	mp3StreamPtr++;
	mp3BytesLeft--;
      }
      return false;
    }

    MP3GetLastFrameInfo(mp3Decoder, &mp3FrameInfo);
    const size_t monoSamples = mp3DownmixToMono(mp3DecodedFrame,
						static_cast<size_t>(mp3FrameInfo.outputSamps),
						mp3FrameInfo.nChans);
    if (monoSamples == 0U) {
      return false;
    }

    return mp3PublishLoopSamples(mp3MonoFrame, monoSamples);
  }

  bool mp3FillUntil(const size_t targetSamples)
  {
    size_t noProgressCount = 0U;
    while (mp3PcmCount.load(std::memory_order_relaxed) < targetSamples && mp3RingFreeSpace() >= mp3MonoFrameSamples) {
      if (mp3DecodeOneFrame()) {
	noProgressCount = 0U;
      } else if (++noProgressCount >= 4U) {
	break;
      }
    }
    return mp3PcmCount.load(std::memory_order_relaxed) != 0U;
  }

  THD_FUNCTION(mp3DecoderThread, arg)
  {
    (void)arg;
    chRegSetThreadName("mp3Decoder");
    while (true) {
      if (activeLoop == nullptr || mp3Active == false || activeLoop->generator != GeneratorType::None) {
	chThdSleepMilliseconds(10);
	continue;
      }

      if (mp3PcmCount.load(std::memory_order_relaxed) < mp3LowWatermark) {
	(void)mp3FillUntil(mp3PrimeSamples);
      } else {
	chThdSleepMilliseconds(2);
      }
    }
  }
}

void Audio::start(void)
{
  gptStart(&GPTD6, &gpt6cfg1);
  dacStart(&DACD1, &dac1cfg1);

  if (not mp3ThreadStarted) {
    chThdCreateStatic(waMp3Decoder, sizeof(waMp3Decoder), mp3DecoderPriority,
		      mp3DecoderThread, nullptr);
    mp3ThreadStarted = true;
  }
}

void Audio::select(const size_t loopIndex)
{
  if ((loopIndex != loop) &&
      (loopIndex < loops.size())) {
    const bool wasPlaying = isPlaying();
    stopDac();
    loop = loopIndex;
    startDac();
    if (wasPlaying) {
      stopTimer();
      startTimer();
    }
  }
}

void Audio::select(const char* sampleName)
{
  auto sample = std::find_if(loops.begin(), loops.end(),
			     [&, sampleName](const auto &s) {
			       return s.name.compare(sampleName) == 0;
			     });
  if (sample != loops.end()) {
    const size_t index = sample - loops.begin();
    select(index);
  }
}

void Audio::play(void)
{
  if (not isPlaying()) {
    startTimer();
  }
}

void Audio::pause(void)
{
  stopTimer();
}

void Audio::setDbVolume(const uint8_t volume)
{
  const uint32_t attn = 25 - (volume/4U);
  const float logAttn = powf(10, attn / 10.0f);
  setAttenuation(1.0f/logAttn);
}

void Audio::startDac(void)
{
  const AudioLoop &audioLoop = loops[loop];
  activeLoop = &audioLoop;

  if (audioLoop.generator == GeneratorType::None) {
    mp3Prepare(audioLoop);
    (void)mp3FillUntil(std::size(dmaBuff));
  } else {
    synthPhase = 0.0f;
    synthModPhase = 0.0f;
  }
  mp3Active = true;

  fillDacBuffer(dmaBuff, std::size(dmaBuff));
  dacStartConversion(&DACD1, &dacgrpcfg1,
		     reinterpret_cast<dacsample_t *>(dmaBuff),
		     std::size(dmaBuff));
}

void Audio::stopDac(void)
{
  mp3Active = false;
  dacStopConversion(&DACD1);
}

void Audio::startTimer(void)
{
  if (GPTD6.state == GPT_READY) {
    gptStopTimer(&GPTD6);
    gptStartContinuous(&GPTD6, DAC_MAX_FREQUENCY / AUDIO_SAMPLE_FREQUENCY);
  }
}

void Audio::stopTimer(void)
{
  if (GPTD6.state != GPT_READY) {
    gptStopTimer(&GPTD6);
  }
}

void Audio::end_cb1(DACDriver *dacp)
{
  custom_dac_sample_t * const dmaBuf = dmaBuff + (dacIsBufferComplete(dacp) ?
						  halfBufferSize :
						  0U);
  fillDacBuffer(dmaBuf, halfBufferSize);
}

void Audio::fillDacBuffer(custom_dac_sample_t *dest, const size_t n)
{
  if (activeLoop != nullptr && activeLoop->generator != GeneratorType::None) {
    const float phaseInc = 500.0f / 32000.0f * 2.0f * std::numbers::pi_v<float>; // 500Hz
    const float modInc = 10.0f / 32000.0f * 2.0f * std::numbers::pi_v<float>;    // 10Hz
    
    for (size_t i = 0; i < n; i++) {
      float sample = std::sin(synthPhase);
      synthPhase += phaseInc;
      if (synthPhase >= 2.0f * std::numbers::pi_v<float>) {
        synthPhase -= 2.0f * std::numbers::pi_v<float>;
      }
      
      if (activeLoop->generator == GeneratorType::Sine500Mod) {
         float mod = (std::sin(synthModPhase) + 1.0f) * 0.5f;
         sample *= mod;
         synthModPhase += modInc;
         if (synthModPhase >= 2.0f * std::numbers::pi_v<float>) {
           synthModPhase -= 2.0f * std::numbers::pi_v<float>;
         }
      }
      
      const float attenuated = 2048.0f + (sample * 2047.0f * attenuation);
      dest[i] = static_cast<custom_dac_sample_t>(std::clamp(attenuated, 0.0f, 4095.0f));
    }
    return;
  }

  for (size_t i = 0; i < n; i++) {
    if (mp3PcmCount.load(std::memory_order_relaxed) == 0U) {
      dest[i] = 2048U;
      continue;
    }

    const decoded_pcm_sample_t sample = mp3PcmRing[mp3PcmReadIdx];
    mp3PcmReadIdx++;
    if (mp3PcmReadIdx == mp3PcmRingSize) {
      mp3PcmReadIdx = 0U;
    }
    mp3PcmCount.fetch_sub(1U, std::memory_order_relaxed);

    const float attenuated = 2048.0f + ((float(sample) * attenuation) / 16.0f);
    dest[i] = static_cast<custom_dac_sample_t>(std::clamp(attenuated, 0.0f, 4095.0f));
  }
}

etl::string_view Audio::getName(const size_t index)
{
  const size_t idx = (index >= loops.size()) ? loop : index;
  return etl::string_view(loops[idx].name);
}

const DACConfig Audio::dac1cfg1 = {
  .init         = 2047U,
  .datamode     = DAC_DHRM_12BIT_RIGHT,
  .cr           = 0,
  .mcr          = 0
};

const DACConversionGroup Audio::dacgrpcfg1 = {
  .num_channels = 1U,
  .end_cb       = &Audio::end_cb1,
  .error_cb     = nullptr,
  .trigger      = DAC_TRG(0)
};

const GPTConfig Audio::gpt6cfg1 = {
  .frequency    = DAC_MAX_FREQUENCY,
  .callback     = NULL,
  .cr2          = TIM_CR2_MMS_1,
  .dier         = 0U
};

size_t Audio::loop = std::numeric_limits<size_t>::max();
float Audio::attenuation = 1.0f;
custom_dac_sample_t IN_DMA_SECTION(Audio::dmaBuff[1024]) = {};
