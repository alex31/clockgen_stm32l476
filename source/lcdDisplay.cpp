#include "lcdDisplay.hpp"
#include "stdutil.h"


/*
adresse of rows :
00-19  
64-83  
20-39
84-104

 */
namespace DP {

  const uint8_t rowAddr[4] = {0U, 64U, 20U, 84U};
  
  const hd44780_pins_t lcdpins = {
				  .RS = LINE_LCD_RS,
				  .RW = LINE_LCD_RW,
				  .E =  LINE_LCD_E,
				  .A =  LINE_DUMMY,
				  .D = {
					LINE_LCD_D4,
					LINE_LCD_D5,
					LINE_LCD_D6,
					LINE_LCD_D7
					}
  };
  
  
  static const HD44780Config lcdcfg = {
				       .cursor = HD44780_CURSOR_OFF,        
  .blinking = HD44780_BLINKING_OFF,    
  .font = HD44780_SET_FONT_5X8,       
  .lines = HD44780_SET_2LINES,        
  .pinmap = &lcdpins,                 
  .backlight = 100,                   
};

}

bool LCDDisplay::init()
{
  DebugTrace("LCDDisplay::init() start");
  hd44780ObjectInit(&lcdd);
  hd44780Start(&lcdd, &DP::lcdcfg);
  hd44780ClearDisplay(&lcdd);
  DebugTrace("LCDDisplay::init() end");
  return true;
}



bool LCDDisplay::loop()
{

  
  return true;
}

void LCDDisplay::draw()
{
  for (size_t lineN =0;lineN < DP::lcdHeight; lineN++) {
    hd44780Write(&lcdd, DP::rowAddr[lineN], fb[lineN].c_str());
  }
}

void LCDDisplay::write(const uint8_t lineN, etl::string_view sv)
{
  if (lineN < DP::lcdHeight) {
    fb[lineN] = etl::string<DP::lcdWide>(sv);
  }
}

void LCDDisplay::write(const uint8_t lineN, const uint8_t posX, etl::string_view sv)
{
  if ((lineN < DP::lcdHeight) and (posX < DP::lcdWide)) {
    fb[lineN].replace(posX, sv.size(), sv.data(), sv.size());
  }
}
