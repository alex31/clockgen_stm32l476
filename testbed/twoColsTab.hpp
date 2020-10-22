#pragma once
#include <array>
#include "lcdTab.hpp"




class TwoColsTab : public LcdTab {
public:
  TwoColsTab (const StateId stateId);
  void enter(void) override;
  void leave(void) override ;
  void eventCb(const Event& ev) override;
private:
  MenuEntries<10, 16> audioSample{"sample", fb, 10, 0, {
				    {1, "hoorn"},
	                            {2, "tone"},
				    {3, "alarm"},
				    {4, "drift"},
				    {5, "siren"},
				    {6, "nuclear"},
				    {7, "fire"}
    }};
  NumericEntry<10> audioVol{"volume", fb, 10, 0, 30, 10, {0, 100}};

  MenuEntries<10, 16> info{"info", fb, 10, 0, {
				   {1, "manuel"},
				   {2, "readme"},
				   {3, "events"}
					       }};
  MenuEntries<10, 16> left{"sample", fb, 0, 0, {}};
  BaseWidget *right = &audioSample;
  //  std::array rights{&audioSample, &audioVol, &info};
  std::array<BaseWidget*, 3> rights{&audioSample, &audioVol, &info};
};
