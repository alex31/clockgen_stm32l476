#include "lcdTab.hpp"

FrameBuffer<LCD_WIDTH, LCD_HEIGHT> LcdTab::fb {};

void LcdTab::propagate(const Event& ev)
{
  LcdTab *active = static_cast<LcdTab *>(IhmState::top());
  chDbgAssert(active != nullptr, "LcdTab::push has not been called at least once");
  active->eventCb(ev);
}

LcdTab* LcdTab::push(const StateId active)
{
  return static_cast<LcdTab *>(IhmState::push(active));
}
