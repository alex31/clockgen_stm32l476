#pragma once

#include <array>
#include <cstdint>
#include "etl/cstring.h"
#include "etl/vector.h"
#include "etl/string_view.h"
#include "etl/span.h"



// g++-10 -Wall -std=c++20 -I../../../../../etl/include/ -I.  menuEntries.cpp
namespace DP {
  constexpr size_t threadStackSize = 1024U;
  constexpr size_t virtualHeight = 16U;
  constexpr size_t lcdWide = 20U;
}

template<size_t W, size_t H>
class FrameeBuffer {
public:
  std::array<etl::string<W>, H>::iterator begin(void) {return fb.begin();}
  std::array<etl::string<W>, H>::iterator end(void) {return fb.end();}
  size_t getHeight(void) {return H;}
  size_t getWide(void) {return W;}
private:
  std::array<etl::string<W>, H> fb;
};

using FrameBuffer = std::array<etl::string<DP::lcdWide>, DP::virtualHeight>;
using FrameBufferView = etl::span<etl::string<DP::lcdWide>>;

using FixedStr = etl::string<10>;

struct Entry {
  int value;
  FixedStr str;
  uint8_t line=0;
  uint8_t posx=0;
};


class MenuEntries {
public:
  MenuEntries(std::initializer_list<Entry> il);
  bool addEntry(const Entry& e);
  const Entry& operator[](const size_t index) const {return entries[index];}
  void fill(const uint8_t margin = 0U, const etl::string_view sep = "");
  void print(void) const;
  static void print(FrameBufferView fbv);
  FrameBufferView getView(const size_t index);

private:
  static constexpr size_t MaxEntries = 20;
  etl::vector<Entry, MaxEntries> entries{};
  FrameBuffer fb{};
};

