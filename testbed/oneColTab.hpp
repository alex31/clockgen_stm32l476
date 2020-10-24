#pragma once
#include "lcdTab.hpp"
#include "menuEntries.hpp"


class OneColTab : public LcdTab {
public:
  OneColTab (const StateId stateId, BaseWidget* w) : LcdTab(stateId),
						     widget(w) {widget->setParentFb(&fb);}
  void enter(void) override;
  void leave(void) override ;
  void eventCb(const Event& ev) override;
private:
  BaseWidget* widget;
};
