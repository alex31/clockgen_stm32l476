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
class FrameBuffer {
public:
  using FbView = etl::span<etl::string<W>>;
  std::array<etl::string<W>, H>::iterator begin(void)  {return fbr.begin();}
  std::array<etl::string<W>, H>::iterator end(void) {return fbr.end();}
  std::array<etl::string<W>, H>::const_iterator begin(void) const {return fbr.cbegin();}
  std::array<etl::string<W>, H>::const_iterator end(void) const {return fbr.cend();}
  etl::string<W>& operator[](const size_t index) {return fbr[index];}
  FbView getView(size_t pos, size_t len);
  template<size_t WF, size_t HF>
  void copyRect(const FrameBuffer<WF, HF> &from, const uint8_t line,
		const uint8_t posx);

  static constexpr size_t getHeight(void) {return H;}
  static constexpr size_t getWide(void) {return W;}
private:
  std::array<etl::string<W>, H> fbr;
};


template<size_t W, size_t H>
FrameBuffer<W, H>::FbView FrameBuffer<W, H>::getView(size_t pos, size_t len) {
    if ((pos + len) < H)
      return FbView{&fbr[pos], len};
    else
      return FbView{&fbr[0], 0};
  }

template<size_t W, size_t H>
template<size_t WF, size_t HF>
void FrameBuffer<W, H>::copyRect(const FrameBuffer<WF, HF>& from, const uint8_t line,
		const uint8_t posx) {
  
}


using FixedStr = etl::string<10>;

struct Entry {
  int value;
  FixedStr str;
  uint8_t line=0;
  uint8_t posx=0;
};


class MenuEntries {
  static constexpr size_t W = 20;
  static constexpr size_t L = 16;

public:
  MenuEntries(std::initializer_list<Entry> il);
  bool addEntry(const Entry& e);
  const Entry& operator[](const size_t index) const {return entries[index];}
  void fill(const uint8_t margin = 0U, const etl::string_view sep = "");
  void print(void) const;
  static void print(FrameBuffer<W, L>::FbView fbv);
  FrameBuffer<W, L>::FbView getView(const size_t index);

private:
  static constexpr size_t MaxEntries = 20;
  etl::vector<Entry, MaxEntries> entries{};
  FrameBuffer<W, L> fb{};
};

