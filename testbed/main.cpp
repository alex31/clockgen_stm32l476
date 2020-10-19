#include <iostream>
#include "menuEntries.hpp"
#include "hardwareConf.hpp"

// g++-10 -Wall -std=c++20 -I../../../../../etl/include/ -I.   main.cpp
// g++-9 -Wall -std=c++2a -I../../../../../etl/include/ -I.   main.cpp


/*
 -> * methode print au niveau de framebuffer
   + la version PC utilise std::cout, la version MCU devra utiliser hd44780 et avoir un pointeur sur un driver lcd
 * up et down appellent fill et copyrect
 * le main appelle le print de framebuffer

 */


int main(void)
{
  FrameBuffer<LCD_WIDTH, LCD_HEIGHT> mainFb;
  MenuEntries<20, 16> me (mainFb, 0, 0, {{1, "1_Hz"},
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

  NumericEntry<20> ne(mainFb, 0, 0, 0, 10, {0, 100});

  for (int i=0; i<2; i++) {
    me.next();
  }

  for (int i=0; i<2; i++) {
    me.prev();
  }

  for (int i=0; i<2; i++) {
    ne.next();
  }

  for (int i=0; i<2; i++) {
    ne.prev();
  }

  
}




