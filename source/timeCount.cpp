#include "timeCount.hpp"
#include "commonRessource.hpp"



bool TimeCount::init()
{
  return true;
}



bool TimeCount::loop()
{
  storage.incAge();
  return true;
}
