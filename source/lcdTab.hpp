#pragma once
#include "ihmState.hpp"
#include "menuEntries.hpp"
#include "hardwareConf.hpp"
#include "event.hpp"
#include "lcdDisplay.hpp"

class LcdTab : public IhmState {
  public:
  LcdTab (const StateId stateId) : IhmState(stateId) {}
  virtual void enter(void) override = 0 ;
  virtual void leave(void) override = 0 ;
  virtual void eventCb(const Event& ev) = 0;
private:
};
