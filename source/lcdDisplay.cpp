#include "lcdDisplay.hpp"
#include "stdutil.h"
#include "adc.hpp"
#include "freqCapture.hpp"
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
    MutexRAII(const MutexRAII&) = delete;
    MutexRAII& operator=(const MutexRAII&) = delete;
  public:
    MutexRAII(mutex_t *_mut) : mut(_mut) {chMtxLock(mut);};
    ~MutexRAII() {chMtxUnlock(mut);};
  private:
    mutex_t *mut;
  };

  constexpr uint8_t rowAddr[4] = {0U, 64U, 20U, 84U};
  constexpr uint8_t heartGlyphBlank = 1U;
  constexpr uint8_t heartGlyphSmall = 2U;
  constexpr uint8_t heartGlyphMedium = 3U;
  constexpr uint8_t heartGlyphLarge = 4U;
  constexpr std::array<std::array<uint8_t, 8>, 4> heartGlyphs = {{
      {{0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000, 0b00000}},
      {{0b00000, 0b00000, 0b01010, 0b00100, 0b00000, 0b00000, 0b00000, 0b00000}},
      {{0b00000, 0b01010, 0b11111, 0b01110, 0b00100, 0b00000, 0b00000, 0b00000}},
      {{0b01010, 0b11111, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000, 0b00000}}
    }};
  constexpr std::array<uint8_t, 10> heartBeatSequence = {{
      heartGlyphBlank,
      heartGlyphSmall,
      heartGlyphMedium,
      heartGlyphLarge,
      heartGlyphMedium,
      heartGlyphSmall,
      heartGlyphMedium,
      heartGlyphLarge,
      heartGlyphMedium,
      heartGlyphBlank
    }};
  
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
  .gpt = {
    .gptd = &GPTD7,
    .frequency = 1000000U,
    .step_delay_us = 3U
  },
  .backlight = 0U
};

}

bool LCDDisplay::init()
{
  hd44780ObjectInit(&lcdd);
  hd44780Start(&lcdd, &DP::lcdcfg);
  hd44780ClearDisplay(&lcdd);
  hd44780CustomGraphic(&lcdd, DP::heartGlyphBlank, DP::heartGlyphs[0].data());
  hd44780CustomGraphic(&lcdd, DP::heartGlyphSmall, DP::heartGlyphs[1].data());
  hd44780CustomGraphic(&lcdd, DP::heartGlyphMedium, DP::heartGlyphs[2].data());
  hd44780CustomGraphic(&lcdd, DP::heartGlyphLarge, DP::heartGlyphs[3].data());

  return true;
}



bool LCDDisplay::loop()
{
  DP::MutexRAII m(&mut);
  const uint8_t glyph = DP::heartBeatSequence[heartBeatIdx++];
  hd44780Write(&lcdd, xy2pos(3,  19), "%c", char(glyph));
  //  hd44780Write(&lcdd, xy2pos(2U, 0U), "ps=%.2f F=%s   ", ADC::getPowerSupplyVoltage(),
  //	       LCDDisplay::freq2Str(ICU::getFrequency()).data());
  //  hd44780Write(&lcdd, xy2pos(3U, 0U), "lv=%.2f", ADC::getLogicVoltage());

  heartBeatIdx %= DP::heartBeatSequence.size();

  //enableCursor(true);
  //setCursorPos(0, 15);

  return true;
}


void LCDDisplay::write(const uint8_t lineN, const uint8_t posX, const char* str)
{
  if ((lineN <  LCD_HEIGHT) and (posX <  LCD_WIDTH)) {
    DP::MutexRAII m(&mut);
    hd44780RawWrite(&lcdd, xy2pos(lineN, posX), str);
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
  line = std::min(line, static_cast<uint8_t>( LCD_HEIGHT - 1U));
  posx = std::min(posx, static_cast<uint8_t>( LCD_WIDTH -1U));
  return DP::rowAddr[line] + posx;
}



etl::string<10> LCDDisplay::freq2Str(uint32_t freq)
{
  char buf[12] = {0};
  
    if (freq == 0) {
      strncpy(buf, "--------", sizeof(buf));
  } else if (freq < 1_khz) {
    snprintf(buf, sizeof(buf), "%03ld Hz", freq);
  } else if (freq < 10_khz) {
    snprintf(buf, sizeof(buf), "%04.2f KHz", freq/1000.0f); 
  } else if (freq < 100_khz) {
    snprintf(buf, sizeof(buf), "%04.1f KHz", freq/1000.0f);
  } else  {
    snprintf(buf, sizeof(buf), "%03ld KHz", freq/1000); 
  }
  return etl::string<10>(buf);
}

etl::string<10> LCDDisplay::time2Str(uint32_t usec)
{
  char buf[10] = {0};
  
  if (usec < 1000) {
    snprintf(buf, sizeof(buf), "%03ld uS", usec);
  } else if (usec < 1000000) {
    snprintf(buf, sizeof(buf), "%04.2f mS", usec / 1000.0f); 
  } else {
    snprintf(buf, sizeof(buf), "%04.2f S", usec / 1000000.0f);
  } 
  return etl::string<10>(buf);
}
 
