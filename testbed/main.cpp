#include <iostream>
#include <array>
#include "menuEntries.hpp"
#include "hardwareConf.hpp"
#include "mainTab.hpp"
#include "twoColsTab.hpp"
#include "oneColTab.hpp"

// g++-10 -Wall -std=c++20 -I../../../../../etl/include/ -I.   *.cpp 

/*

   + la version PC utilise std::cout, la version MCU devra utiliser hd44780 et avoir un pointeur sur un driver lcd

   * tester avec un menuEntry à gauche et un mix menuEntries et NumericEntry à droite

 */



#ifdef HIGH_LEVEL_TEST
int main(void)
{
  MenuEntries<10, 16> audioSample{"sample", 10, 0, {
				    {1, "hoorn"},
	                            {2, "tone"},
				    {3, "alarm"},
				    {4, "drift"},
				    {5, "siren"},
				    {6, "nuclear"},
				    {7, "fire"}
    }};
  NumericEntry<10> audioVol{"volume", 10, 0, 30, 10, {0, 100}};

  MenuEntries<10, 16> info{"info", 10, 0, {
				   {1, "manuel"},
				   {2, "readme"},
				   {3, "events"}
					       }};

  MenuEntries<20, 16> frequencies{"freq.", 0, 0, {{1, "1_Hz"},
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
 
  MainTab mt(StateId::Freq);
  OneColTab sc(StateId::FreqShortCut, &frequencies);
  TwoColsTab tc(StateId::Info, {&audioSample, &audioVol, &info});
 
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
#endif



#ifdef LOW_LEVEL_TEST
int main(void)
{
  FrameBuffer<LCD_WIDTH, LCD_HEIGHT> mainFb;
  //                                   02345678901234567890
  FrameBuffer<LCD_WIDTH, 12U> readme{{"-------README-------",
				      "tourner pour augmen-",
				      "-er ou baisser la   ",
				      "fréquence pour      ",
				      "F1 en haut et F2 en ",
				      "bas. Un appui court ",
				      "permet de parcourir ",
				      "rapidement la gamme ",
				      "dans le sens de la  ",
				      "fleche. Un appui    ",
				      "long donne acces au ",
				      "menu des raccourcis "}};
  
  MenuEntries<10, 16> me ("freq", &mainFb, 0, 0, {{1, "1_Hz"},
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

  NumericEntry<10> ne("vol", &mainFb, 10, 0, 30, 10, {0, 100});
  std::array<BaseWidget*, 2> arr = {&me, &ne};
  ScrollText st("readme", &mainFb, &readme);
  
  st.draw();
  for (size_t i=0; i< 10; i++)
    st.next();
  for (size_t i=0; i< 10; i++)
    st.prev();



  for (int i=0; i<2; i++) {
    for (auto &w : arr)  {
      w->next();
    }
  
     for (auto &w : arr)  {
      w->prev();
    }
  }

  mainFb.print();
  // arr[0]->next();
  // arr[1]->next();


}
#endif


