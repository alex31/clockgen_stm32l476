#pragma once
#include "lcdTab.hpp"


class MainTab : public LcdTab {
public:
  MainTab (const StateId stateId) : LcdTab(stateId) {}
  void enter(void) override;
  void leave(void) override ;
  void eventCb(const Event& ev) override;
};
