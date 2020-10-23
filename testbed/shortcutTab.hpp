#pragma once
#include "lcdTab.hpp"
#include "menuEntries.hpp"


class ShortcutTab : public LcdTab {
public:
  ShortcutTab (const StateId stateId) : LcdTab(stateId) {
    me.bind([](int32_t val) {
	      std::cout << "new ShortcutTab value = " << val << std::endl;
	    });
  }
  void enter(void) override;
  void leave(void) override ;
  void eventCb(const Event& ev) override;
private:
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
  
};
