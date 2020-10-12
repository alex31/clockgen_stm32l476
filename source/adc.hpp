#pragma once
#include "workerClass.hpp"
#include "hardwareConf.hpp"

namespace {
  constexpr size_t threadStackSize = 1024U;
}


class ADC : public WorkerThread<ADC> {
public:
  ADC(const tprio_t m_prio) :
    WorkerThread("adc", threadStackSize, m_prio)
  {};
  static float getPowerSupplyVoltage(void)  {return psVolt;}
  static float getLogicVoltage(void)  {return logicVolt;}
private:
  friend WorkerThread<ADC>;
  bool init(void) final;
  bool loop(void) final;
  void proceedUp(void);
  void proceedDown(void);
  static float psVolt;
  static float logicVolt;
};


