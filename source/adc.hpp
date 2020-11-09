#pragma once
#include "workerClass.hpp"
#include "hardwareConf.hpp"
#include "event.hpp"

namespace {
  constexpr size_t threadStackSize = 1024U;
}


class ADC : public WorkerThreadSingleton<ADC> {
public:
  enum EventSource {None, Logic, PowerSupply};

 ADC(ADC const &) = delete;
 ADC& operator=(ADC const &) = delete;

  static float getPowerSupplyVoltage(void)  {return psVolt;}
  static float getLogicVoltage(void)  {return logicVoltage;}
  static Event getVoltageHealth(void);
protected:
  ADC(const tprio_t m_prio) :
    WorkerThreadSingleton("adc", threadStackSize, m_prio)
  {};
private:
  ~ADC() {}
  friend WorkerThreadSingleton<ADC>;
  friend WorkerThread<ADC>;
  bool init(void) final;
  bool loop(void) final;
  static float psVolt;
  static float logicVoltage;
};


