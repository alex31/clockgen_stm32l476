#pragma once
#include "ihmState.hpp"
#include "menuEntries.hpp"
#include "hardwareConf.hpp"
#include "event.hpp"

class LcdTab : public IhmState {
  public:
  LcdTab (const StateId stateId) : IhmState(stateId) {}
  virtual void enter(void) override = 0 ;
  virtual void leave(void) override = 0 ;
  virtual void eventCb(const Event& ev) = 0;
  void print(void) {fb.print();}
  static void propagate(const Event& ev);
  static LcdTab* push(const StateId active);
  
protected:
  static FrameBuffer<LCD_WIDTH, LCD_HEIGHT> fb;
};
