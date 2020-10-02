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

using namespace std::literals;

static constexpr uint32_t operator"" _hz (unsigned long long int freq)
{
  return freq;
}
static constexpr uint32_t operator"" _khz (unsigned long long int freq)
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

static_assert(BOUTON_F2_IN_A_TIM == BOUTON_F2_IN_B_TIM);
static_assert(BOUTON_F1_IN_A_TIM == BOUTON_F1_IN_B_TIM);
static_assert(BOUTON_F1_IN_A_TIM_CH == 1);
static_assert(BOUTON_F2_IN_B_TIM_CH == 2);
static_assert(BOUTON_F1_IN_A_TIM_CH == 1);
static_assert(BOUTON_F2_IN_B_TIM_CH == 2);
static_assert(FREQMETER_IN_TIM_CH <= 2);
static_assert((FREQMETER_IN_TIM == 2) or (FREQMETER_IN_TIM == 5));

static constexpr PWMDriver &PWM_F1 = CONCAT(PWMD, CLOCK_F1_OUT_TIM);
static constexpr PWMDriver &PWM_F2 = CONCAT(PWMD, CLOCK_F2_OUT_TIM);
static constexpr ICUDriver &ICU_IN = CONCAT(ICUD, FREQMETER_IN_TIM);

static inline    stm32_tim_t * const ENCODER_F1 = CONCAT(STM32_TIM, BOUTON_F1_IN_A_TIM);
static inline    stm32_tim_t * const ENCODER_F2 = CONCAT(STM32_TIM, BOUTON_F2_IN_A_TIM);
