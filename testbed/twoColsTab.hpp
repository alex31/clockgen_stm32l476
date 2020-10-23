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
  template<typename ...Args> 
  TwoColsTab (const StateId stateId, Args ...args);
  void enter(void) override;
  void leave(void) override ;
  void eventCb(const Event& ev) override;
private:
  BaseWidget *right = nullptr;
  //  std::array rights{&audioSample, &audioVol, &info};
  etl::vector<BaseWidget*, 16> rights;
  MenuEntries<10, 16> left{"left", &fb, 0, 0, {}};
};


template<typename ...Args>
TwoColsTab::TwoColsTab (const StateId stateId, Args ...args) : LcdTab(stateId)
{
  (..., rights.push_back(args));
  static_assert(sizeof...(args) != 0);
  
  right = rights[0];
  
  for (size_t i=0; i< rights.size(); i++) {
    rights[i]->setParentFb(&fb);
    left.addEntry({int(i), rights[i]->getName()});
  }
  left.bind([this] (int32_t val) {
	      right =  rights[val];
	      right->draw();
	    });
}



