#include <iostream>
#include <array>
#include "menuEntries.hpp"
#include "hardwareConf.hpp"
#include "mainTab.hpp"
#include "shortcutTab.hpp"
#include "twoColsTab.hpp"

// g++-10 -Wall -std=c++20 -I../../../../../etl/include/ -I.   *.cpp 

/*

   + la version PC utilise std::cout, la version MCU devra utiliser hd44780 et avoir un pointeur sur un driver lcd

   * tester avec un menuEntry à gauche et un mix menuEntries et NumericEntry à droite

 */




int main(void)
{
  MainTab mt(StateId::Freq);
  ShortcutTab sc(StateId::FreqShortCut);
  TwoColsTab tc(StateId::Info);
  std::cout << "tc obj = " << &tc << std::endl;
 
  LcdTab::push(StateId::Freq);
  LcdTab::propagate({.event = Events::Turn, .idx=0, .load=142});
  LcdTab::propagate({.event = Events::LongClick, .idx=0, .load=0});
  LcdTab::propagate({.event = Events::Turn, .idx=0, .load=1});
  LcdTab::propagate({.event = Events::Turn, .idx=0, .load=1});
  LcdTab::propagate({.event = Events::Turn, .idx=0, .load=-1});
  LcdTab::propagate({.event = Events::Turn, .idx=0, .load=-1});
  LcdTab::propagate({.event = Events::ShortClick, .idx=0, .load=0});

  LcdTab::propagate({.event = Events::DoubleClick, .idx=0, .load=0});
  LcdTab::propagate({.event = Events::Turn, .idx=1, .load=1});
  LcdTab::propagate({.event = Events::Turn, .idx=0, .load=1});
  LcdTab::propagate({.event = Events::Turn, .idx=1, .load=-1});
  LcdTab::propagate({.event = Events::Turn, .idx=0, .load=-1});
  LcdTab::propagate({.event = Events::ShortClick, .idx=0, .load=0});
}


































#ifdef LOW_LEVEL_TEST
int main(void)
{
  FrameBuffer<LCD_WIDTH, LCD_HEIGHT> mainFb;
  MenuEntries<10, 16> me (mainFb, 0, 0, {{1, "1_Hz"},
					 {20, "20_Hz"},
					 {300, "300_hz"},
					 {400, "400_hz"},
					 {2400, "2.4_Khz"},
					 {5000, "5_Khz"},
					 {8000, "8_Khz"},
					 {10000, "10_Khz"},
					 {19200, "19.2_Khz"},
					 {36400, "36.4_Khz"}
    });

  NumericEntry<10> ne(mainFb, 10, 0, 30, 10, {0, 100});

  std::array<BaseWidget*, 2> arr = {&me, &ne};
  
  for (auto &w : arr)  {
    for (int i=0; i<2; i++) {
      w->next();
    }
  
    for (int i=0; i<2; i++) {
      w->prev();
    }
  }

  mainFb.write(1,0, "toto est %s", "rentré");
  mainFb.print();
  // arr[0]->next();
  // arr[1]->next();


}
#endif


