#pragma once
#include "etl/vector.h"
#include "lcdTab.hpp"
#include "menuEntries.hpp"
#include "hardwareConf.hpp"

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
  etl::vector<BaseWidget*, TWO_COLS_MAX_LEFT_ENTRIES> rights;
  MenuEntries<10, TWO_COLS_MAX_LEFT_ENTRIES> left{"left", &fb, 0, 0, {}};
};





