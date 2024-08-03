#pragma once

#include "ch.h"
#include "hal.h"
#include <algorithm>

/*
 * Concatenate preprocessor tokens A and B after macro-expanding them.
 */
#define CONCAT_NX(st1, st2) st1 ## st2
#define CONCAT(st1, st2) CONCAT_NX(st1, st2)


/*
#                 ______   _____         
#                /  ____| |_   _|        
#                | (___     | |          
#                 \___ \    | |          
#                .____) |  _| |_         
#                \_____/  |_____|        
#                                                              _      _                   
#                                                             | |    (_)                  
#                  ___    ___    _ __   __   __   ___   _ __  | |_    _     ___    _ __   
#                 / __|  / _ \  | '_ \  \ \ / /  / _ \ | '__| | __|  | |   / _ \  | '_ \  
#                | (__  | (_) | | | | |  \ V /  |  __/ | |    \ |_   | |  | (_) | | | | | 
#                 \___|  \___/  |_| |_|   \_/    \___| |_|     \__|  |_|   \___/  |_| |_| 
*/

static constexpr uint32_t operator"" _hz (unsigned long long int freq)
{
  return freq;
}
static constexpr uint32_t operator"" _khz (unsigned long long int freq)
{
  return freq * 1000UL;
}
static constexpr uint32_t operator"" _khz (long double freq)
{
  return freq * 1000UL;
}
static constexpr uint32_t operator"" _mhz (unsigned long long int freq)
{
  return freq * 1000_khz;
}
static constexpr long double operator"" _ohm (long double resistance)
{
  return resistance;
}
static constexpr long double operator"" _kohm (long double resistance)
{
  return resistance * 1000UL;
}
static constexpr uint32_t operator"" _percent (unsigned long long int freq)
{
  return freq * 100UL;
}

/*
#                 _                          _                                           
#                | |                        | |                                          
#                | |__     __ _   _ __    __| |  __      __   __ _   _ __    ___         
#                | '_ \   / _` | | '__|  / _` |  \ \ /\ / /  / _` | | '__|  / _ \        
#                | | | | | (_| | | |    | (_| |   \ V  V /  | (_| | | |    |  __/        
#                |_| |_|  \__,_| |_|     \__,_|    \_/\_/    \__,_| |_|     \___|        
#                                               _                     _            _            
#                                              | |                   (_)          | |           
#                  ___    ___    _ __    ___   | |_    _ __    __ _   _    _ __   | |_          
#                 / __|  / _ \  | '_ \  / __|  | __|  | '__|  / _` | | |  | '_ \  | __|         
#                | (__  | (_) | | | | | \__ \  \ |_   | |    | (_| | | |  | | | | \ |_          
#                 \___|  \___/  |_| |_| |___/   \__|  |_|     \__,_| |_|  |_| |_|  \__|         
*/


static constexpr float VCC_33 = 3.3f;
static constexpr size_t ADC_RESOLUTION_IN_BITS = 12U;
static constexpr uint32_t SAMPLE_MAX = (1<<ADC_RESOLUTION_IN_BITS) - 1;
static constexpr float LOGIC_VOLTAGE_RATIO = 2.0f;
static constexpr float POWER_SUPPLY_VOLTAGE_RATIO = 5.55;
static constexpr systime_t TIMOUT_BEFORE_POWER_LOSS_LOG = TIME_MS2I(1000U);


static_assert(BOUTON_F2_IN_A_TIM == BOUTON_F2_IN_B_TIM);
static_assert(BOUTON_F1_IN_A_TIM == BOUTON_F1_IN_B_TIM);
static_assert(BOUTON_F1_IN_A_TIM_CH == 1);
static_assert(BOUTON_F2_IN_B_TIM_CH == 2);
static_assert(BOUTON_F1_IN_A_TIM_CH == 1);
static_assert(BOUTON_F2_IN_B_TIM_CH == 2);
static_assert(FREQMETER_IN_TIM_CH <= 2);
static_assert((FREQMETER_IN_TIM == 2) or (FREQMETER_IN_TIM == 5));
static_assert(MEM_SCL_I2C == MEM_SDA_I2C);

static constexpr PWMDriver &PWM_F1 = CONCAT(PWMD, CLOCK_F1_OUT_TIM);
static constexpr PWMDriver &PWM_F2 = CONCAT(PWMD, CLOCK_F2_OUT_TIM);

static constexpr ICUDriver &ICU_IN = CONCAT(ICUD, FREQMETER_IN_TIM);

static inline     stm32_tim_t * const ENCODER_F1 = CONCAT(STM32_TIM, BOUTON_F1_IN_A_TIM);
static inline     stm32_tim_t * const ENCODER_F2 = CONCAT(STM32_TIM, BOUTON_F2_IN_A_TIM);

static constexpr I2CDriver &I2C_FRAM = CONCAT(I2CD, MEM_SCL_I2C);
static constexpr uint32_t I2C_FAST_400KHZ_DNF3_R200NS_F50NS_PCLK80MHZ_TIMINGR = 0x10B0133C;
static constexpr uint32_t I2C_FASTPLUS_1MHZ_DNF3_R100NS_F50NS_PCLK80MHZ_TIMINGR  = 0x00B00B25;
static constexpr uint32_t stm32_cr1_dnf(const uint32_t n) {
  return (n & 0x0f) << 8;
}

static constexpr I2CConfig i2ccfg_400 = {
  .timingr = I2C_FAST_400KHZ_DNF3_R200NS_F50NS_PCLK80MHZ_TIMINGR, // Refer to the STM32F7 reference manual
  .cr1 =  stm32_cr1_dnf(3U), // Digital noise filter activated (timingr should be aware of that)
  .cr2 = 0 // Only the ADD10 bit can eventually be specified here (10-bit addressing mode)
} ;

static constexpr I2CConfig i2ccfg_1000 = {
  .timingr = I2C_FASTPLUS_1MHZ_DNF3_R100NS_F50NS_PCLK80MHZ_TIMINGR, // Refer to the STM32F7 reference manual
  .cr1 = stm32_cr1_dnf(3U), // Digital noise filter activated (timingr should be aware of that)
  .cr2 = 0 // Only the ADD10 bit can eventually be specified here (10-bit addressing mode)
} ;

static constexpr size_t LCD_WIDTH = 20U;
static constexpr size_t LCD_HEIGHT = 4U;
static constexpr size_t TWO_COLS_MAX_LEFT_ENTRIES = 16U;

/*
#                                                    
#                                                    
#                 _   _   ___     ___   _ __         
#                | | | | / __|   / _ \ | '__|        
#                | |_| | \__ \  |  __/ | |           
#                 \__,_| |___/   \___| |_|           
#                            _    _    _              _       _                 
#                           | |  (_)  | |            | |     | |                
#                  ___    __| |   _   | |_     __ _  | |__   | |    ___         
#                 / _ \  / _` |  | |  | __|   / _` | | '_ \  | |   / _ \        
#                |  __/ | (_| |  | |  \ |_   | (_| | | |_) | | |  |  __/        
#                 \___|  \__,_|  |_|   \__|   \__,_| |_.__/  |_|   \___|        
*/

static constexpr uint32_t  ANTI_REBOUND_INTERVAL_MS = 30;
static constexpr uint32_t  LONG_CLIC_INTERVAL_MS = 500;
static constexpr uint32_t  DOUBLE_CLIC_INTERVAL_MS = 100;
static constexpr bool	   NUMERIC_ENTRY_SHOW_INTERVAL = false;
static constexpr uint32_t  KNOB_INTERVAL_MS = 100;
