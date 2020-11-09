#include "timeCount.hpp"
#include "storage.hpp"



bool TimeCount::init()
{
  return true;
}



bool TimeCount::loop()
{
  Storage::instance().incAge();
  return true;
}
