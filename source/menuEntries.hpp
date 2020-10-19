#pragma once

#include <array>
#include <cstdint>
#include "etl/string.h"
#include "etl/vector.h"
#include "etl/string_view.h"
#include "etl/span.h"


template<size_t W, size_t H>
class FrameBuffer {
  template<size_t WF, size_t HF>
  friend class FrameBuffer;
  
public:
  using FbView = etl::span<etl::string<W>>;
  std::array<etl::string<W>, H>::iterator begin(void)  {return fbr.begin();}
  std::array<etl::string<W>, H>::iterator end(void) {return fbr.end();}
  std::array<etl::string<W>, H>::const_iterator begin(void) const {return fbr.cbegin();}
  std::array<etl::string<W>, H>::const_iterator end(void) const {return fbr.cend();}
  etl::string<W>& operator[](const size_t index) {return fbr[index];}
  FbView getView(size_t line, size_t len);
  template<size_t WF, size_t HF>
  void copyRect(const FrameBuffer<WF, HF> &from, const uint8_t line,
		const uint8_t posx);
  template<size_t WF>
  void copyRect(const etl::span<etl::string<WF>>, const uint8_t line,
		const uint8_t posx);

  static constexpr size_t getHeight(void) {return H;}
  static constexpr size_t getWide(void) {return W;}
  
private:
  std::array<etl::string<W>, H> fbr;
};


template<size_t W, size_t H>
FrameBuffer<W, H>::FbView FrameBuffer<W, H>::getView(size_t line, size_t len) {
    if (line < H)
      return FbView{&fbr[line], std::min(len, H-line)};
    else
      return FbView{&fbr[0], 0};
  }

template<size_t W, size_t H>
template<size_t WF, size_t HF>
void FrameBuffer<W, H>::copyRect(const FrameBuffer<WF, HF>& from, const uint8_t line,
		const uint8_t posx) {
  if ((line >= H) or (posx >= W))
    return;
  size_t fromLine =0U;
  for (size_t l = line; l < std::min(line + HF, H); l++) {
    fbr[l].append(20, ' ');
    fbr[l].replace(posx, WF, from.fbr[fromLine++]);
  }
}

template<size_t W, size_t H>
template<size_t WF>
void FrameBuffer<W, H>::copyRect(const etl::span<etl::string<WF>> from, const uint8_t line,
		const uint8_t posx) {
  if ((line >= H) or (posx >= W))
    return;
  size_t fromLine =0U;
  for (size_t l = line; l < std::min(line + from.size(), H); l++) {
    fbr[l].append(20, ' ');
    fbr[l].replace(posx, WF, from[fromLine++]);
  }
}


using FixedStr = etl::string<10>;

struct Entry {
  int value;
  FixedStr str;
  uint8_t line=0;
  uint8_t posx=0;
};



class BaseEntry {
public:
  virtual void next(void) = 0;
  virtual void prev(void) = 0;
};



class MenuEntries : public BaseEntry {
  static constexpr size_t W = LCD_WIDTH;
  static constexpr size_t L = 16;
  
public:
  MenuEntries(std::initializer_list<Entry> il);
  bool addEntry(const Entry& e);
  const Entry& operator[](const size_t index) const {return entries[index];}
  void fill(const uint8_t margin = 0U, const etl::string_view sep = "");
  void setSelect(uint8_t s) {selectedItem = s;}
  uint8_t getSelect(void) const {return selectedItem;}
  void next(void) override {selectedItem = (selectedItem+1U) % entries.size();}
  void prev(void) override {selectedItem = selectedItem==0U ? entries.size() - 1U :
                                                           selectedItem - 1U ;}
  // v1= (v1==0) ? 2 : (v1-1U)%3U;
  void print(void) const;
  static void print(FrameBuffer<W, L>::FbView fbv);
  FrameBuffer<W, L>::FbView getView(const size_t index);

private:
  static constexpr size_t MaxEntries = 20;
  etl::vector<Entry, MaxEntries> entries{};
  uint8_t selectedItem = 0U;
  FrameBuffer<W, L> fb{};
};

class NumericEntry : public BaseEntry {
  static constexpr size_t W = 10;
  static constexpr size_t L = 1;
  
public:
  NumericEntry(const int32_t _val, const int32_t _inc,
	       const std::pair<int32_t, int32_t>& _interval) :
    val(_val),
    inc(_inc),
    interval(_interval) {};
  void fill(const uint8_t margin = 0U, const etl::string_view sep = "");
  uint8_t getVal(void) const {return val;}
  void next(void) override {val += inc; val=std::clamp(val, interval.first, interval.second);}
  void prev(void) override {val -=inc; val=std::clamp(val, interval.first, interval.second);}

  void print(void) const;
  FrameBuffer<W, L>::FbView getView();

private:
  int32_t val = 0U;
  int32_t inc = 0U;
  const std::pair<int32_t, int32_t> interval;
  FrameBuffer<W, L> fb{};
};

