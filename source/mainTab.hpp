#pragma once
#include "lcdTab.hpp"
#include "clockGenerator.hpp"
#include "hardwareConf.hpp"
#include "freqCapture.hpp"
#include "frequencyTuner.hpp"


class MainTab : public LcdTab {
  enum class Direction {Up=0x7E, Down=0x7F};
  struct Frequency {
    uint32_t lastFreq;
    uint32_t freq;
    ClockGenerator &cg;
    Direction dir;
    FrequencyTuner tuner;
  };
  

  
public:
  MainTab (const StateId stateId);
  void enter(void) override;
  void leave(void) override ;
  void eventCb(const Event& ev) override;
  void setFreq(const uint32_t freq);
  void enableF2(bool en);

private:
  ClockGenerator f1{&PWM_F1, CLOCK_F1_OUT_TIM_CH - 1};
  ClockGenerator f2{&PWM_F2, CLOCK_F2_OUT_TIM_CH - 1};
  Frequency frequencies[2] = {{1, 1, f1, Direction::Up, {}}, {1, 1, f2, Direction::Up, {}}};

  uint8_t selFreq=0;
  static uint32_t applyFreqTarget(Frequency &frequency, uint32_t targetFreq);
  void draw(void);
};
