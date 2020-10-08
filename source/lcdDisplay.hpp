#pragma once
#include "workerClass.hpp"
#include "hardwareConf.hpp"
#include "ch.h"
#include "hal.h"
#include "hd44780.h"
#include "etl/cstring.h"
#include "etl/string_view.h"
#include <array>

namespace DP {
  constexpr size_t threadStackSize = 1536U;
  constexpr size_t lcdHeight = 4U;
  constexpr size_t lcdWide = 20U;
}


class LCDDisplay : public WorkerThread<LCDDisplay> {
public:
  LCDDisplay(const tprio_t m_prio) :
    WorkerThread("lcdDisplay", DP::threadStackSize, m_prio)
  {}

  void draw(void);
  //  void write(const uint8_t lineN, etl::string_view sv);
  void write(const uint8_t lineN, const uint8_t posX, etl::string_view sv);
  void write(const uint8_t lineN, const uint8_t posX, const char* fmt, ...);
  void enableCursor(const bool enable);
  void setCursorPos(uint8_t line, uint8_t posx);
  static etl::string<10> freq2Str(uint32_t freq);
  static etl::string<10> time2Str(uint32_t usec);
  
private:
  friend WorkerThread<LCDDisplay>;

  std::array<etl::string<DP::lcdWide+1>, DP::lcdHeight> fb{};
  signed char heartBeat = ' ';
  
  HD44780Driver lcdd;
  MUTEX_DECL(mut);

  bool init(void) final;
  bool loop(void) final;
  static constexpr uint8_t xy2pos(uint8_t line, const uint8_t posx);
};


