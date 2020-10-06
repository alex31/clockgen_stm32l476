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
  constexpr size_t threadStackSize = 1024U;
  constexpr size_t lcdHeight = 4U;
  constexpr size_t lcdWide = 20U;
}


class LCDDisplay : public WorkerThread<LCDDisplay> {
public:
  LCDDisplay(const tprio_t m_prio) :
    WorkerThread("pushButton", DP::threadStackSize, m_prio)
  {}

  void draw(void);
  void write(const uint8_t lineN, etl::string_view sv);
  void write(const uint8_t lineN, const uint8_t posX, etl::string_view sv);
  void enableCursor(const bool enable);
  void setCursorPos(uint8_t line, uint8_t posx);
  
private:
  friend WorkerThread<LCDDisplay>;

  std::array<etl::string<DP::lcdWide>, DP::lcdHeight> fb{};
  signed char heartBeat = ' ';
  
  HD44780Driver lcdd;
  MUTEX_DECL(mut);

  bool init(void) final;
  bool loop(void) final;
  static constexpr uint8_t xy2pos(uint8_t line, const uint8_t posx);
};


