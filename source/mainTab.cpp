#include "mainTab.hpp"
#include "adc.hpp"
#include "stdutil.h"
#include "storage.hpp"
#include "beepIn.hpp"

MainTab::MainTab(const StateId stateId) : LcdTab(stateId)
{
  Storage &storage = Storage::instance();
  storage.load();
  frequencies[0].freq = storage.getFrequencies()[0];
  frequencies[1].freq = storage.getFrequencies()[1];
  (void) f1.setFreq(frequencies[0].freq);
  (void) f2.setFreq(frequencies[1].freq);
  enableF2(storage.getEnableF2());
}

void MainTab::enter(void)
{
  draw();
}

void MainTab::enableF2(bool en)
{
  if (en)
    f2.play();
  else
    f2.pause();
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


void MainTab::eventCb(const Event& ev) 
{
  auto & [lastFreq, freq, cg, dir] = frequencies[ev.getIndex()];
  int32_t inc=0;
  
  const uint32_t mulExp = std::clamp(static_cast<uint32_t>(log10f(freq)), 2UL, 5UL) -2UL;
 
  switch (ev.getEvent()) {
  case  Events::Undo : {
    freq = lastFreq;
    break;
  }
    
  case  Events::Turn : {
    const int32_t delta = ev.getLoad();
    const int32_t sign = delta > 0 ? 1 : -1;
    const int32_t deltabs = std::abs(delta);
    lastFreq = freq;

    switch (deltabs) {
    case 0: break;
    case 1 : 
    case 2 :  inc = sign; break;
    case 3 :  inc = 3*sign; break;
    default : inc  = sign * powf((deltabs-1)*2, 1.0f+(deltabs/5.0f)); break;
    }

    const uint32_t minFreqInRange = powf(10.0f, floorf(log10f(freq))) -1.0f;
    inc = std::clamp(inc, -200L, 200L);
    freq += inc * powf(10, mulExp);
    freq = std::clamp(freq, minFreqInRange, 999_khz);
    freq -= freq % static_cast<uint32_t>(powf(10, mulExp));

    dir = freq >= lastFreq ? Direction::Up : Direction::Down;

    
    break;
  }
  case  Events::SetFreq : {
    lastFreq = freq;
    freq = ev.getLoad();
    dir = freq >= lastFreq ? Direction::Up : Direction::Down;
    break;
  }
    
  case Events::ShortClick : {
    //    DebugTrace("lastFreq=%lu freq=%lu mulExp=%lu dir=%s", lastFreq, freq, mulExp,
    //	       dir == Direction::Up ? "UP" : "DOWN");
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
    
    freq = cg.setFreq(std::clamp(freq, 1_hz, 980_khz));
    if (freq > 200_khz) {
      uint32_t mul = 1U;
      if (dir == Direction::Up)
	while ((freq < 980_khz) and (freq <= lastFreq)) {
	  freq = cg.setFreq(std::clamp(freq + (1_khz*mul++), 1_hz, 980_khz));
	  //	DebugTrace("Iter freq UP = %lu", freq);
	}
      else
	while ((freq > 1_hz) and (freq >= lastFreq)) {
	  freq = cg.setFreq(std::clamp(freq - (1_khz*mul++), 1_hz, 980_khz));
	  //	DebugTrace("Iter freq DOWN = %lu", freq);
	}
    }
    Storage::instance().setFrequency(ev.getIndex(), freq);
  }

  
  
  draw();
}



