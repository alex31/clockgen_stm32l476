#include "mainTab.hpp"
#include "adc.hpp"
#include "stdutil.h"
#include "storage.hpp"
#include "beepIn.hpp"

namespace {
  constexpr FrequencyTunerConfig freqTunerCfg = {
    .dtIgnoreUs = 500U,
    .dtIdleResetUs = 300'000U,
    .tauSpeedUs = 50'000.0,
    .speedMaxTicksPerSec = 100.0,
    .speedDeadOnTicksPerSec = 8.0,
    .speedDeadOffTicksPerSec = 6.0,
    .speedFullTicksPerSec = 50.0,
    .speedGainMax = 1.6,
    .tauContinuityUs = 220'000.0,
    .continuityRisePerTick = 0.25,
    .continuityGainMax = 0.8,
    .reverseContinuityDrop = 0.15,
    .reverseSpeedDrop = 0.5,
    .speedExtraStepsMax = 20.0,
    .continuityExtraStepsMax = 6.0,
    .maxStepsPerTick = 32U
  };

  constexpr uint32_t displayQuantumHz(const uint32_t freq)
  {
    if (freq < 1_khz) {
      return 1U;
    }
    if (freq < 10_khz) {
      return 10U;
    }
    if (freq < 100_khz) {
      return 100U;
    }
    return 1000U;
  }

  constexpr uint32_t roundToDisplayQuantum(const uint32_t freq,
                                           const uint32_t quantumHz)
  {
    return ((freq + (quantumHz / 2U)) / quantumHz) * quantumHz;
  }

  uint32_t stepDisplayedFrequency(const uint32_t currentFreq,
                                  const bool up,
                                  const uint32_t visibleSteps)
  {
    const uint32_t quantumHz = displayQuantumHz(currentFreq);
    const uint32_t anchorFreq = roundToDisplayQuantum(currentFreq, quantumHz);
    const uint32_t clampedSteps = std::max<uint32_t>(1U, visibleSteps);
    const uint32_t deltaHz = clampedSteps * quantumHz;

    if (up) {
      return std::min<uint32_t>(980_khz, anchorFreq + deltaHz);
    }

    if (anchorFreq <= deltaHz) {
      return 1_hz;
    }
    return std::max<uint32_t>(1_hz, anchorFreq - deltaHz);
  }
}

MainTab::MainTab(const StateId stateId) : LcdTab(stateId)
{
  Storage &storage = Storage::instance();
  storage.load();
  frequencies[0].freq = storage.getFrequencies()[0];
  frequencies[1].freq = storage.getFrequencies()[1];
  (void) f1.setFreq(frequencies[0].freq);
  (void) f2.setFreq(frequencies[1].freq);
  frequencies[0].tuner.init(freqTunerCfg, frequencies[0].freq);
  frequencies[1].tuner.init(freqTunerCfg, frequencies[1].freq);
  enableF2(storage.getEnableF2());
}

void MainTab::enter(void)
{
  draw();
}

void MainTab::enableF2(bool en)
{
  if (en)
    f2.enableOutput(en);
  else
    f2.pause();

  f2.enableOutput(en);
}

void MainTab::draw(void)
{
  if (IhmState::top() != this)
    return;
  Storage &storage = Storage::instance();
  
  fb.write(0,0, "V=%.2f   ", ADC::getLogicVoltage());
  fb.write(7,0, "F1=%s %c%*c",
	   LCDDisplay::freq2Str(frequencies[0].freq).c_str(), char(frequencies[0].dir),
	   LCD_WIDTH-7, ' ');

  if (Storage::instance().hasFailed() == true) {
    fb.write(0,1, "sto fail%*c", 10, ' ');
  } else if (BeepIn::hasRecentChange()) {
    fb.write(0,1, "Vol=%d  ", storage.getVolume());
  } else {
    fb.write(0,1, "%*c", 10, ' ');
  }
  
  if (storage.getEnableF2()) {
    fb.write(7,1, "F2=%s %c%*c",
	     LCDDisplay::freq2Str(frequencies[1].freq).c_str(), char(frequencies[1].dir),
	     LCD_WIDTH-7, ' ');
  } else {
    fb.write(7,1, "%*c", LCD_WIDTH-7, ' ');
  }

  if (ICU::getFrequency() == 0) {
    fb.write(0,2, "%*c", LCD_WIDTH, ' ');
    fb.write(0,3, "%*c", LCD_WIDTH, ' ');
  } else {
    fb.write(0,2, "%*c", LCD_WIDTH, ' ');
    fb.write(7,2, "FIn= %s%*c",
	     LCDDisplay::freq2Str(ICU::getFrequency()).c_str(),
	     LCD_WIDTH-7, ' ');
    fb.write(0,3, "D=%.2f%*c", ICU::getDuty(),
	     7, ' ');
    fb.write(7,3, "W=%s%*c",
	     LCDDisplay::time2Str(ICU::getPulseWidthUsec()).c_str(),
	     LCD_WIDTH-7, ' ');
  }
  
  print();
}


void MainTab::leave(void) 
{
}

void MainTab::setFreq(const uint32_t freq)
{
  if (selFreq < 2) {
    eventCb({Events::SetFreq, selFreq, int(freq)});
  }
}

uint32_t MainTab::applyFreqTarget(Frequency &frequency, uint32_t targetFreq)
{
  auto &[lastFreq, freq, cg, dir, tuner] = frequency;
  (void)tuner;
  freq = cg.setFreq(std::clamp(targetFreq, 1_hz, 980_khz));

  if (freq > 200_khz) {
    uint32_t mul = 1U;
    if (dir == Direction::Up) {
      while ((freq < 980_khz) and (freq <= lastFreq)) {
        freq = cg.setFreq(std::clamp(freq + (1_khz * mul++), 1_hz, 980_khz));
      }
    } else {
      while ((freq > 1_hz) and (freq >= lastFreq)) {
        freq = cg.setFreq(std::clamp(freq - (1_khz * mul++), 1_hz, 980_khz));
      }
    }
  }

  return freq;
}


void MainTab::eventCb(const Event& ev) 
{
  auto & [lastFreq, freq, cg, dir, tuner] = frequencies[ev.getIndex()];
  const uint32_t nowUs = chTimeI2US(chVTGetSystemTimeX());
 
  switch (ev.getEvent()) {
  case  Events::Undo : {
    freq = lastFreq;
    break;
  }
    
  case  Events::Turn : {
    lastFreq = freq;
    dir = ev.getLoad() >= 0 ? Direction::Up : Direction::Down;
    const uint32_t visibleStride = tuner.onEncoderEvent(
        ev.getLoad() >= 0 ? TuneDirection::Positive : TuneDirection::Negative,
        nowUs);
    freq = stepDisplayedFrequency(freq, dir == Direction::Up, visibleStride);
    break;
  }
  case  Events::SetFreq : {
    lastFreq = freq;
    freq = ev.getLoad();
    dir = freq >= lastFreq ? Direction::Up : Direction::Down;
    break;
  }
    
  case Events::ShortClick : {
    const uint32_t mulExp =
        std::clamp(static_cast<uint32_t>(log10f(freq)), 2UL, 5UL) - 2UL;
    lastFreq = freq;
    if (dir == Direction::Up) {
      if (mulExp != 3) {
	freq = powf(10, ceilf(log10f(freq+1)));
      }
    } else { // dir == Direction::Down
      if (freq != 1_hz)
	freq = powf(10, floorf(log10f(freq-1)));
    }
    break;
  }

 
  case Events::LongClick :
    selFreq =  ev.getIndex();
    IhmState::push(StateId::FreqShortCut);
    break;

  case Events::DoubleClick :
    if (ev.getIndex() == 0)
      IhmState::push(StateId::Param);
    else
      IhmState::push(StateId::Manual);
    break;

  case Events::ClickAtPowerOn :
    IhmState::push(StateId::VoltageChoice);
    break;

  case Events::UnderVoltage :
  case Events::OverVoltage :
    IhmState::push(StateId::VoltageAlert);
    break;
    
  default : break;
  }

  if ((ev.getEvent() == Events::Turn) or
      (ev.getEvent() == Events::ShortClick) or
      (ev.getEvent() == Events::Undo) or
      (ev.getEvent() == Events::SetFreq)) {
    freq = applyFreqTarget(frequencies[ev.getIndex()], freq);
    Storage::instance().setFrequency(ev.getIndex(), freq);
  }

  
  
  draw();
}
