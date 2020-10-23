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


#ifdef PR
  MenuEntries<20, 16> me{"freq.", &fb, 0, 0, {{1, "1_Hz"},
	                            {20, "20_Hz"},
				    {300, "300_hz"},
				    {400, "400_hz"},
				    {2400, "2.4_Khz"},
				    {5000, "5_Khz"},
				    {8000, "8_Khz"},
				    {10000, "10_Khz"},
				    {19200, "19.2_Khz"},
				    {36400, "36.4_Khz"}
    }};
#endif
