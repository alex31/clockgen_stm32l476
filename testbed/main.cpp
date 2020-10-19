#include <iostream>
#include "menuEntries.hpp"
#include "hardwareConf.hpp"

// g++-10 -Wall -std=c++20 -I../../../../../etl/include/ -I.  menuEntries.cpp main.cpp
// g++-9 -Wall -std=c++2a -I../../../../../etl/include/ -I.  menuEntries.cpp main.cpp


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

  for (int i=0; i<10; i++) {
    me.next();
  }

  for (int i=0; i<10; i++) {
    me.prev();
  }

  
}





#ifdef OLD_STUFF
  me.fill(5, "| ");
  me.print();
  std::cout << "#################" << std::endl;
  //  MenuEntries::print(me.getView(4));

  std::cout << "#################" << std::endl;
  FrameBuffer<5,5> fb55;
  FrameBuffer<2,4> fb22;
  fb55[0] = fb55[1] = fb55[2] = fb55[3] = fb55[4] = "12345";
  fb22[0] = fb22[1] = fb22[2] = fb22[3] = "AB";
  fb55.copyRect(fb22.getView(0,2), 2, 2);

  for (const auto& str : fb55)
    std::cout << str.data() << std::endl;

  return 0;
#endif
