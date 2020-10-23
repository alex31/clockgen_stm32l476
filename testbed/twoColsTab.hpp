#pragma once
#include "etl/vector.h"
#include "lcdTab.hpp"

/*
template<typename ...Args> auto sum2(Args ...args) 
{ 
    return (args + ...);
}
*/


class TwoColsTab : public LcdTab {
public:
  TwoColsTab (const StateId stateId, std::initializer_list<BaseWidget *> il);
  void enter(void) override;
  void leave(void) override ;
  void eventCb(const Event& ev) override;
private:
  BaseWidget *right = nullptr;
  //  std::array rights{&audioSample, &audioVol, &info};
  etl::vector<BaseWidget*, 16> rights;
  MenuEntries<10, 16> left{"left", &fb, 0, 0, {}};
};





