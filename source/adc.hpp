#pragma once
#include "workerClass.hpp"
#include "hardwareConf.hpp"
#include "event.hpp"

namespace {
  constexpr size_t threadStackSize = 1024U;
}


class ADC : public WorkerThread<ADC> {
public:
  enum EventSource {None, Logic, PowerSupply};
  ADC(const tprio_t m_prio) :
    WorkerThread("adc", threadStackSize, m_prio)
  {};
  static float getPowerSupplyVoltage(void)  {return psVolt;}
  static float getLogicVoltage(void)  {return logicVoltage;}
  static Event getVoltageHealth(void);
private:
  friend WorkerThread<ADC>;
  bool init(void) final;
  bool loop(void) final;
  static float psVolt;
  static float logicVoltAverage;
  static float logicVoltage;
};


