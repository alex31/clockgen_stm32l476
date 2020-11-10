#include "storage.hpp"
#include "fram.hpp"
#ifndef NOSHELL
#include "stdutil.h"
#endif


bool Storage::load(void)
{
  bool ret;
  if ((ret = FRAM::read(*this, 0)) == true) {
    if (magic != MAGIC) {
      magic = MAGIC;
      frequencies = {1U,1U};
      volume = 50U;
      sampleIndex = 0U;
      voltageRef = 5.0f;
      age = 0;
      underVoltageAlert = 0;
      overVoltageAlert = 0;
      i2cFailure = 0;
      powerOn = 0;
      store();
    }
  } else {
    i2cFailAtInit = true;
  }
  // DebugTrace("====== READ ========");
  // print();
  // DebugTrace("\n");
  return ret;
}

bool Storage::store(void)
{
  //  DebugTrace("====== STORE ========");
  //print();
  //  DebugTrace("\n");
  return (not i2cFailAtInit) and FRAM::write(*this, 0);
}

void Storage::print(void)
{
#ifndef NOSHELL
  DebugTrace("magic = 0x%lx", magic);
  DebugTrace("frq = %lu, %lu", frequencies[0], frequencies[1]);
  DebugTrace("voltageRef = %.2f", voltageRef);
  DebugTrace("age = %lu", age);
  DebugTrace("power on = %lu", powerOn);
  DebugTrace("underVoltageAlert = %lu", underVoltageAlert);
  DebugTrace("overVoltageAlert = %lu", overVoltageAlert);
  DebugTrace("volume = %u", volume);
  DebugTrace("sampleIndex = %u", sampleIndex);
#endif
}


bool Storage::i2cFailAtInit = false;
