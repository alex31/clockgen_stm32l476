#pragma once

#include "hardwareConf.hpp"
#include <array>
#include <cstdint>
#include "etl/string.h"
#include "etl/vector.h"
#include "etl/string_view.h"
#include "etl/span.h"
#include <functional>
#include "ihmState.hpp"


#ifdef SIM_PC
#include <iostream>
#else
#include "lcdDisplay.hpp"
#endif

class FrameBufferBase {
public:
  using printFn_t = void(*)(uint8_t posx, uint8_t posy, const char* str);
  static void setPrintFn(printFn_t fun) {printFn = fun;};
  
protected:
  static void doprint(uint8_t posx, uint8_t posy, const char*);
  
private:
  static printFn_t	printFn;
};


template<size_t W, size_t H>
class FrameBuffer : public FrameBufferBase {
  using LineStr = etl::string<W>;
  
  template<size_t WF, size_t HF>
  friend class FrameBuffer;
  
public:
  using FbView = etl::span<etl::string<W>>;
  
  FrameBuffer() = default;
  FrameBuffer(std::initializer_list<const etl::string<W>> il);
  std::array<etl::string<W>, H>::iterator begin(void)  {return fbr.begin();}
  std::array<etl::string<W>, H>::iterator end(void) {return fbr.end();}
  std::array<etl::string<W>, H>::const_iterator begin(void) const {return fbr.cbegin();}
  std::array<etl::string<W>, H>::const_iterator end(void) const {return fbr.cend();}
  etl::string<W>& operator[](const size_t index) {return fbr[index];}
  void print(void) const;
  template<typename... Args>
  FrameBuffer<W, H>& write(const uint8_t posX, const uint8_t posY, const char* fmt, Args... args);
  template<typename... Args>
  FrameBuffer<W, H>& append(const uint8_t posY, const char* fmt, Args... args);
  FbView getView(size_t posy, size_t len);
  template<size_t WF, size_t HF>
  void copyRect(const FrameBuffer<WF, HF> &from, const uint8_t posx,
		const uint8_t posy);
  template<size_t WF>
  void copyRect(const etl::span<etl::string<WF>>, const uint8_t posx,
		const uint8_t posy);

  static constexpr size_t getHeight(void) {return H;}
  static constexpr size_t getWide(void) {return W;}
  
private:
  std::array<LineStr, H> fbr;
};

template<size_t W, size_t H>
FrameBuffer<W, H>::FrameBuffer(std::initializer_list<const etl::string<W>> il)
{
  size_t i=0U;
  for (const auto& s : il) {
    fbr[i++] = s;
  }
}


template<size_t W, size_t H>
FrameBuffer<W, H>::FbView FrameBuffer<W, H>::getView(size_t posy, size_t len) {
  if (posy >= H)
    abort();
  
  return FbView{&fbr[posy], std::min(len, H-posy)};
}

template<size_t W, size_t H>
template<size_t WF, size_t HF>
void FrameBuffer<W, H>::copyRect(const FrameBuffer<WF, HF>& from, const uint8_t posx,
		const uint8_t posy) {
  if ((posx >= W) or (posy >= H))
    abort();
  size_t fromPosy =0U;
  for (size_t y = posy; y < std::min(posy + HF, H); y++) {
    fbr[y].append(W, ' ');
    auto fstr =  from.fbr[fromPosy];
    fstr.append(WF, ' ');
    fbr[y].replace(posx, WF, fstr);
    fromPosy++;
  }
}

template<size_t W, size_t H>
template<size_t WF>
void FrameBuffer<W, H>::copyRect(const etl::span<etl::string<WF>> from, const uint8_t posx,
		const uint8_t posy) {
  if ((posx >= W) or (posy >= H))
    abort();
  size_t fromPosy =0U;
  for (size_t y = posy; y < std::min(posy + from.size(), H); y++) {
    fbr[y].append(W, ' ');
    auto fstr =  from[fromPosy];
    fstr.append(WF, ' ');
    fbr[y].replace(posx, WF, fstr);
    fromPosy++;
  }
}

template<size_t W, size_t H>
void FrameBuffer<W, H>::print(void) const
{
  size_t i=0;
  for (const auto& s : fbr) {
    doprint(0, i++, s.data());
  }
  doprint(0, i++, "\n");
}


template<size_t W, size_t H>
template<typename... Args>
FrameBuffer<W, H>& FrameBuffer<W, H>::write(const uint8_t posX, const uint8_t posY, const char* fmt, Args... args)
{
  constexpr size_t len = LCD_WIDTH+1;
  char buf[len];
  snprintf(buf, len, fmt, std::forward<Args&&>(args)...);
  const size_t slen = std::min(len-posX, strlen(buf));
  fbr[posY].replace(posX, slen, buf, slen);
  return *this;
}

template<size_t W, size_t H>
template<typename... Args>
FrameBuffer<W, H>& FrameBuffer<W, H>::append(const uint8_t posY, const char* fmt, Args... args)
{
  return write(strlen(fbr[posY].c_str()), posY, fmt, std::forward<Args&&>(args)...);
}

