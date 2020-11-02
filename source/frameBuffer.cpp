#include "menuEntries.hpp"

FrameBufferBase::printFn_t FrameBufferBase::printFn = nullptr;


void FrameBufferBase::doprint(uint8_t posx, uint8_t posy, const char* str)
{
  if (printFn != nullptr) 
    printFn(posx, posy, str);
}
