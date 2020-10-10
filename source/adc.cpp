#include "adc.hpp"
#include "event.hpp"
#include "hardwareConf.hpp"


#define SMPR1_PS CONCAT(ADC_SMPR1_SMP_AN, SUPPLY_VOLTAGE_ADC_IN)
#define SMPR1_LOGIC CONCAT(ADC_SMPR1_SMP_AN, LOGIC_VOLTAGE_ADC_IN)
#define CHANNEL_PS CONCAT(ADC_CHANNEL_IN, SUPPLY_VOLTAGE_ADC_IN)
#define CHANNEL_LOGIC CONCAT(ADC_CHANNEL_IN, LOGIC_VOLTAGE_ADC_IN)
 

namespace {
  constexpr uint32_t CHANNELS = 2;
  constexpr uint32_t DEPTH = 2;
  constexpr uint32_t ADC_OVERSAMPLING = ADC_SMPR_SMP_640P5;

 constexpr ADCConversionGroup adcgrpcfg1 = {
  .circular = false,
  .num_channels = CHANNELS,
  .end_cb = nullptr,
  .error_cb = nullptr,
  .cfgr = 0U,
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

  adcsample_t adcSamples[CHANNELS * DEPTH];
}

bool ADC::init()
{
   adcStart(&ADCD1, NULL);
   return true;
}



bool ADC::loop()
{
  adcConvert(&ADCD1, &adcgrpcfg1, adcSamples, DEPTH);
  psVolt = (adcSamples[0] + adcSamples[1]) * 3.3f / 4095.0f;
  logicVolt = (adcSamples[1] + adcSamples[3]) * 3.3f / 4095.0f;
  
  return true;
}


float ADC::psVolt=0.0f;
float ADC::logicVolt=0.0f;
