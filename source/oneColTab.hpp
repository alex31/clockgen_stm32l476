#pragma once
#include "lcdTab.hpp"
#include "menuEntries.hpp"


class OneColTab : public LcdTab {
public:
  OneColTab (const StateId stateId, BaseWidget* w,
	     StateId goOnLong = StateId::None,
	     StateId goOnTP = StateId::None) : LcdTab(stateId),
					       widget(w),
					       goOnLongClick(goOnLong),
					       goOnTurnPress(goOnTP)
				     {widget->setParentFb(&fb);}
  void enter(void) override;
  void leave(void) override ;
  void eventCb(const Event& ev) override;
  int  get(void) {return widget->get();}
private:
  BaseWidget* widget;
  StateId goOnLongClick = StateId::None;
  StateId goOnTurnPress = StateId::None;
};
