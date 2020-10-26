#include "mainTab.hpp"
#include "adc.hpp"
#include "fram.hpp"
#include "stdutil.h"
#include "commonRessource.hpp"

MainTab:: MainTab(const StateId stateId) : LcdTab(stateId)
{
  constexpr uint32_t MAGIC = 0xDEADBEEF;
  uint32_t magic;

  if (FRAM::read(magic, 0) == false) {
    DebugTrace("I²C Failed");
    (void) f1.setFreq(1U);
    (void) f2.setFreq(1U);
  } else if (magic == MAGIC) {
    DebugTrace("fram initialized");
    FRAM::read(frequencies[0].freq, 4);
    FRAM::read(frequencies[1].freq, 8);
    (void) f1.setFreq(frequencies[0].freq);
    (void) f2.setFreq(frequencies[1].freq);
  } else {
    DebugTrace("first run : fram NOT initialized");
    FRAM::write(MAGIC, 0);
  }
 
}

void MainTab::enter(void)
{
  draw();
}

void MainTab::draw(void)
{
  fb.write(0,0, "V=%.2f   ", adc.getLogicVoltage());
  fb.write(8,0, "F1= %s %c%*c",
	   LCDDisplay::freq2Str(frequencies[0].freq).c_str(), char(frequencies[0].dir),
	   LCD_WIDTH-8, ' ');

  fb.write(0,1, "%*c", 10, ' ');
  fb.write(8,1, "F2= %s %c%*c",
	   LCDDisplay::freq2Str(frequencies[1].freq).c_str(), char(frequencies[1].dir),
	   LCD_WIDTH-8, ' ');


  if (ICU::getFrequency() == 0) {
    fb.write(0,2, "%*c", LCD_WIDTH, ' ');
    fb.write(0,3, "%*c", LCD_WIDTH, ' ');
  } else {
    fb.write(0,2, "%*c", LCD_WIDTH, ' ');
    fb.write(8,2, "FIn= %s%*c",
	     LCDDisplay::freq2Str(ICU::getFrequency()).c_str(),
	     LCD_WIDTH-8, ' ');
    fb.write(0,3, "D=%.2f%*c", ICU::getDuty(),
	     8, ' ');
    fb.write(8,3, "W=%s%*c",
	     LCDDisplay::time2Str(ICU::getPulseWidthUsec()).c_str(),
	     LCD_WIDTH-8, ' ');
  }
  
  print();
}


void MainTab::leave(void) 
{
}


void MainTab::eventCb(const Event& ev) 
{
  auto & [lastFreq, freq, cg, dir] = frequencies[ev.getIndex()];
  int32_t inc=0;
  
  const uint32_t mulExp = std::clamp(static_cast<uint32_t>(log10f(freq)), 2UL, 5UL) -2UL;
 
  switch (ev.getEvent()) {
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
    default : inc  = sign * powf((deltabs-2)*2, 1.0f+(deltabs/6.0f)); break;
    }

    const uint32_t minFreqInRange = powf(10.0f, floorf(log10f(freq))) -1.0f;
    inc = std::clamp(inc, -200L, 200L);
    freq += inc * powf(10, mulExp);
    freq = std::clamp(freq, minFreqInRange, 999_khz);
    freq -= freq % static_cast<uint32_t>(powf(10, mulExp));

    dir = freq >= lastFreq ? Direction::Up : Direction::Down;

    
    break;
  }
    
  case Events::ShortClick : {
    //    DebugTrace("lastFreq=%lu freq=%lu mulExp=%lu dir=%s", lastFreq, freq, mulExp,
    //	       dir == Direction::Up ? "UP" : "DOWN");
    lastFreq = freq;
    if (dir == Direction::Up) {
      if (mulExp == 3) {
	freq = 1_hz;
      } else {
	freq = powf(10, ceilf(log10f(freq+1)));
      }
    } else { // dir == Direction::Down
      if (freq == 1)
	freq = 100_khz;
      else
	freq = powf(10, floorf(log10f(freq-1)));
    }
    break;
  }

 
  case Events::LongClick :
    IhmState::push(StateId::FreqShortCut);
    break;

  case Events::DoubleClick :
    if (ev.getIndex() == 0)
      IhmState::push(StateId::Info);
    else
      IhmState::push(StateId::Readme);
    break;
 
   
  default : break;
  }

  if ((ev.getEvent() == Events::Turn) or (ev.getEvent() == Events::ShortClick)) {
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
    FRAM::write(freq, 4+(ev.getIndex()*4));
  }
  
  draw();
}



