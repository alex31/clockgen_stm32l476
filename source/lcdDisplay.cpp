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
  class MutexRAII
  {
  public:
    MutexRAII(mutex_t *_mut) : mut(_mut) {chMtxLock(mut);};
    ~MutexRAII() {chMtxUnlock(mut);};
  private:
    mutex_t *mut;
  };

  constexpr uint8_t rowAddr[4] = {0U, 64U, 20U, 84U};
  
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
  .blinking = HD44780_BLINKING_ON,    
  .font = HD44780_SET_FONT_5X8,       
  .lines = HD44780_SET_2LINES,        
  .pinmap = &lcdpins,
  .backlight = 0U
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
  {
    DP::MutexRAII m(&mut);
    hd44780Write(&lcdd, xy2pos(DP::lcdHeight - 1, DP::lcdWide - 1), "%c", heartBeat++);
  }
  if (heartBeat < 0) {
    heartBeat = ' ';
  }
  enableCursor(true);
  setCursorPos(0, 15);

  return true;
}

void LCDDisplay::draw()
{
  DP::MutexRAII m(&mut);
  for (size_t lineN =0;lineN < DP::lcdHeight; lineN++) {
    hd44780Write(&lcdd, DP::rowAddr[lineN], fb[lineN].c_str());
  }
}

void LCDDisplay::write(const uint8_t lineN, etl::string_view sv)
{
  if (lineN < DP::lcdHeight) {
    fb[lineN] = etl::string<DP::lcdWide>(sv);
    std::fill(fb[lineN].begin() + strlen(sv.data()), fb[lineN].end(), ' ');
  }
}

void LCDDisplay::write(const uint8_t lineN, const uint8_t posX, etl::string_view sv)
{
  if ((lineN < DP::lcdHeight) and (posX < DP::lcdWide)) {
    const size_t slen = strlen(sv.data());
    fb[lineN].replace(posX, slen, sv.data(), slen);
  }
}

void LCDDisplay::enableCursor(const bool enable)
{
  DP::MutexRAII m(&mut);
  hd44780ShowCursor(&lcdd, enable);
}

void LCDDisplay::setCursorPos(uint8_t line, uint8_t posx)
{
  DP::MutexRAII m(&mut);
  hd44780SetAddress(&lcdd, xy2pos(line, posx));
}

constexpr uint8_t LCDDisplay::xy2pos(uint8_t line, uint8_t posx)
{
  line = std::min(line, static_cast<uint8_t>(DP::lcdHeight - 1U));
  posx = std::min(posx, static_cast<uint8_t>(DP::lcdWide -1U));
  return DP::rowAddr[line] + posx;
}
