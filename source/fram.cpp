#include "fram.hpp"
#include "ch.h" 
#include "hal.h" 
#include "hardwareConf.hpp"
#include "storage.hpp"

bool FRAM::init(void)
{
  i2cStart(&I2C_FRAM, &i2ccfg_1000);
  return true;
}

void  FRAM::resetI2c(I2CDriver *i2cd)
{
  i2cStop(i2cd);
  i2cStart(i2cd, &i2ccfg_1000);
  Storage::instance().incI2cFailure();
}
