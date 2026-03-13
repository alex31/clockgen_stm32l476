#include "adc.hpp"
#include "hardwareConf.hpp"
#include "stdutil.h"
#include "storage.hpp"

#define SMPR1_PS CONCAT(ADC_SMPR1_SMP_AN, SUPPLY_VOLTAGE_ADC_IN)
#define SMPR1_LOGIC CONCAT(ADC_SMPR1_SMP_AN, LOGIC_VOLTAGE_ADC_IN)
#define CHANNEL_PS CONCAT(ADC_CHANNEL_IN, SUPPLY_VOLTAGE_ADC_IN)
#define CHANNEL_LOGIC CONCAT(ADC_CHANNEL_IN, LOGIC_VOLTAGE_ADC_IN)
 

namespace {
  constexpr uint32_t CHANNELS = 2;
  constexpr uint32_t DEPTH = 128;
  constexpr uint32_t ADC_OVERSAMPLING = ADC_SMPR_SMP_640P5;

 constexpr ADCConversionGroup adcgrpcfg1 = {
  .circular = false,
  .num_channels = CHANNELS,
  .end_cb = nullptr,
  .error_cb = nullptr,
  .cfgr = ADC_CFGR_CONT,
  .cfgr2 = 0U,
  .tr1 = ADC_TR_DISABLED,
  .tr2 = ADC_TR_DISABLED,
  .tr3 = ADC_TR_DISABLED,
  .awd2cr       = 0U,
  .awd3cr       = 0U,
  .smpr = {
	   SMPR1_PS(ADC_OVERSAMPLING) |
	   SMPR1_LOGIC(ADC_OVERSAMPLING)},
  .sqr =   {
	    ADC_SQR1_SQ1_N(CHANNEL_PS) | ADC_SQR1_SQ2_N(CHANNEL_LOGIC),
	    0,
	    0
	    }
 };

  adcsample_t IN_DMA_SECTION_NOINIT(adcSamples[CHANNELS * DEPTH]);
}

bool ADC::init()
{
  DebugTrace("ADC::init()");
  adcStart(&ADCD1, NULL);
  return true;
}



bool ADC::loop()
{
  adcConvert(&ADCD1, &adcgrpcfg1, adcSamples, DEPTH);
  float mean[CHANNELS] {};
  Storage &storage = Storage::instance();
  
  for (size_t d=0; d < CHANNELS * DEPTH; d += CHANNELS)
    for (size_t c=0; c < CHANNELS; c++)
      mean[c] += adcSamples[d+c];
  
  psVolt = mean[0] * VCC_33 * POWER_SUPPLY_VOLTAGE_RATIO / SAMPLE_MAX / DEPTH;
  logicVoltage = mean[1] * VCC_33 * LOGIC_VOLTAGE_RATIO /
    SAMPLE_MAX / DEPTH;
  
  if (Event ev = getVoltageHealth(); ev.getEvent() != Events::None)
    chMBPostTimeout(&EVT::mb, ev.getEventAsMsg(), TIME_INFINITE);
  else
    storage.setVoltageRef(storage.getVoltageRef() * 0.999f +
			  (logicVoltage * 0.001f));
  
  return true;
}



Event ADC::getVoltageHealth(void)
{
  Event ev;
  static constexpr float logicDiffPercentMaxSup = 10;
  static constexpr float logicDiffPercentMaxInf = BUILD_IS_DEBUG ? -10 : -5;
  static constexpr float max12VPSvoltage = 13.5f;
  static constexpr float min12VPSvoltage = 10.5f;

  const float logicDiffPercent = ((logicVoltage * 100.0f) /
				  Storage::instance().getVoltageRef())
                                  - 100.0f;
  if (logicDiffPercent > logicDiffPercentMaxSup)  {
    ev.set(Events::OverVoltage, Logic);
  } else  if (logicDiffPercent < logicDiffPercentMaxInf)  {
    ev.set(Events::UnderVoltage, Logic);
  }  else if (psVolt > max12VPSvoltage) {
    ev.set(Events::OverVoltage, PowerSupply);
  } else  if (psVolt < min12VPSvoltage)  {
    ev.set(Events::UnderVoltage, PowerSupply);
  }
  if (ev.getEvent() != Events::None) {
    DebugTrace("logicVoltage=%.2f, logicDiffPercent=%.2f",
   	       logicVoltage, logicDiffPercent);
  }
  
  return ev;
}

float ADC::psVolt = 0.0f;
float ADC::logicVoltage = 0.0f;
