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
      voltageAlert = 0;
      store();
    }
  } else {
    failure = true;
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
  return FRAM::write(*this, 0);
}

void Storage::print(void)
{
#ifndef NOSHELL
  DebugTrace("magic = 0x%lx", magic);
  DebugTrace("frq = %lu, %lu", frequencies[0], frequencies[1]);
  DebugTrace("voltageRef = %.2f", voltageRef);
  DebugTrace("age = %lu", age);
  DebugTrace("voltageAlert = %lu", voltageAlert);
  DebugTrace("volume = %u", volume);
  DebugTrace("sampleIndex = %u", sampleIndex);
#endif
}


Storage* Storage::singletonCheck = nullptr;
bool Storage::failure = false;
