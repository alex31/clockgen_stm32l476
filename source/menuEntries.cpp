#include "menuEntries.hpp"

FrameBufferBase::printFn_t FrameBufferBase::printFn = nullptr;


void FrameBufferBase::doprint(uint8_t posx, uint8_t posy, const char* str)
{
#ifdef SIM_PC
  assert(printFn != nullptr);
#else
  chDbgAssert(printFn != nullptr, "please call setPrintFn");
#endif
    
  printFn(posx, posy, str);
}
