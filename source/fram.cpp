#include "fram.hpp"
#include "ch.h" 
#include "hal.h" 
#include "hardwareConf.hpp"

bool FRAM::init(void)
{
  i2cStart(&I2C_FRAM, &i2ccfg_1000);
  return true;
}
