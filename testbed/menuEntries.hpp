#pragma once

#include "hardwareConf.hpp"
#include <array>
#include <cstdint>
#include <iostream>
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
  void print(void);
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
  std::array<etl::string<W>, H> fbr;
};


template<size_t W, size_t H>
FrameBuffer<W, H>::FbView FrameBuffer<W, H>::getView(size_t posy, size_t len) {
    if (posy < H)
      return FbView{&fbr[posy], std::min(len, H-posy)};
    else
      return FbView{&fbr[0], 0};
  }

template<size_t W, size_t H>
template<size_t WF, size_t HF>
void FrameBuffer<W, H>::copyRect(const FrameBuffer<WF, HF>& from, const uint8_t posx,
		const uint8_t posy) {
  if ((posx >= H) or (posx >= W))
    return;
  size_t fromPosx =0U;
  for (size_t l = posx; l < std::min(posx + HF, H); l++) {
    fbr[l].append(W, ' ');
    fbr[l].replace(posx, WF, from.fbr[fromPosx++]);
  }
}

template<size_t W, size_t H>
template<size_t WF>
void FrameBuffer<W, H>::copyRect(const etl::span<etl::string<WF>> from, const uint8_t posx,
		const uint8_t posy) {
  if ((posx >= H) or (posx >= W))
    return;
  size_t fromPosx =0U;
  for (size_t l = posx; l < std::min(posx + from.size(), H); l++) {
    fbr[l].append(W, ' ');
    fbr[l].replace(posx, WF, from[fromPosx++]);
  }
}

template<size_t W, size_t H>
void FrameBuffer<W, H>::print(void) 
{
  for (const auto& s : fbr) {
    std::cout << s.data() << std::endl;
  }
  std::cout << std::endl;
}


using FixedStr = etl::string<10>;

struct Entry {
  int value;
  FixedStr str;
  uint8_t posx=0;
  uint8_t posy=0;
};


template <size_t SW, size_t SL>
class BaseEntry {
public:
  BaseEntry(FrameBuffer<LCD_WIDTH, LCD_HEIGHT> &fb,
	    uint8_t _anchorx, uint8_t _anchory) : parentFb(fb),
						  anchorx(_anchorx),
						  anchory(_anchory) {}
  virtual void next(void) = 0;
  virtual void prev(void) = 0;
  virtual void fill(const uint8_t margin = 0U, const etl::string_view sep = "") = 0;
  virtual FrameBuffer<SW, SL>::FbView getView(void) = 0;
  void draw(void);
protected:
  
  FrameBuffer<LCD_WIDTH, LCD_HEIGHT>& parentFb;
  const uint8_t anchorx=0;
  const uint8_t anchory=0;
};


template <size_t SW, size_t SL>
class MenuEntries : public BaseEntry<SW, SL> {

  
public:
  MenuEntries(FrameBuffer<LCD_WIDTH, LCD_HEIGHT> &fb,
	      uint8_t _anchorx, uint8_t _anchory,
	      std::initializer_list<Entry> il);
  bool addEntry(const Entry& e);
  const Entry& operator[](const size_t index) const {return entries[index];}
  void fill(const uint8_t margin = 0U, const etl::string_view sep = "") override;
  void setSelect(uint8_t s) {selectedItem = s;}
  uint8_t getSelect(void) const {return selectedItem;}
  void next(void) override;
  void prev(void) override;
  FrameBuffer<SW, SL>::FbView getView(void) override;
  void print(void) const;

private:

  static constexpr size_t MaxEntries = 20;
  etl::vector<Entry, MaxEntries> entries{};
  uint8_t selectedItem = 0U;
  FrameBuffer<SW, SL> fb{};
};

#ifdef PLUS_TARD
class NumericEntry : public BaseEntry {
  static constexpr size_t W = 10;
  static constexpr size_t L = 1;
  
public:
  NumericEntry(FrameBuffer<LCD_WIDTH, LCD_HEIGHT> &fb,
		uint8_t anchorx, uint8_t anchory,const int32_t _val, const int32_t _inc,
	       const std::pair<int32_t, int32_t>& _interval) :
	       BaseEntry(fb, anchorx, anchory),
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
#endif


/*
#                 _____               _ __    _          
#                |_   _|             | '_ \  | |         
#                  | |    _ __ ___   | |_) | | |         
#                  | |   | '_ ` _ \  | .__/  | |         
#                 _| |_  | | | | | | | |     | |         
#                |_____| |_| |_| |_| |_|     |_|         
*/
template <size_t SW, size_t SL>
void BaseEntry<SW, SL>::draw(void)
{
  fill();
  parentFb.copyRect(getView(), anchorx, anchory);
  parentFb.print();
}

template <size_t SW, size_t SL>
FrameBuffer<SW, SL>::FbView MenuEntries<SW, SL>::getView(void)
{
  int bgin = 0;
  if (selectedItem < entries.size()) {
    bgin = int(entries[selectedItem].posy) - 1;
    bgin = std::clamp(bgin, 0, entries.back().posy - (int(LCD_HEIGHT) - 1));
  }
  //  std::cout << "DBG> LAST (" << entries.back().str.c_str() << ") y=" << int(entries.back().posy) << std::endl;
  //  std::cout << "DBG> bgin=" << bgin << std::endl;
  return fb.getView(bgin, LCD_HEIGHT);
}

template <size_t SW, size_t SL>
MenuEntries<SW, SL>::MenuEntries(FrameBuffer<LCD_WIDTH, LCD_HEIGHT> &fb,
			  uint8_t anchorx, uint8_t anchory,
				 std::initializer_list<Entry> il) : BaseEntry<SW, SL>(fb, anchorx, anchory)
{
  for (const auto& e : il)
    addEntry(e);
  BaseEntry<SW, SL>::draw();
}


template <size_t SW, size_t SL>
bool MenuEntries<SW, SL>::addEntry(const Entry& e)
{
  if (entries.full())
    return false;
  
  entries.push_back({
		     .value = e.value,
		     .str = (FixedStr(" ") += e.str) += " ",
		     .posx = e.posx,
		     .posy = e.posy
    });

  return true;
}

template <size_t SW, size_t SL>
void MenuEntries<SW, SL>::fill(const uint8_t margin, const etl::string_view sep)
{
  uint8_t posy=0U;
  for (auto& s : fb) {
    s.clear();
    s.insert(s.begin(), margin, ' ');
    if (sep.length())
      s.append(sep.data());
  }

  for (size_t index=0; auto& e : entries) {
    auto str = e.str;
    if (index == selectedItem) {
      str.replace(str.begin(),str.begin()+1, "("); 
      str.replace(str.end()-1, str.end(), ")"); 
    }
    if (str.length() >= fb[posy].available()) {
      posy++;
      if (posy >= fb.getHeight())
	break;
    }
    const uint8_t posx = strlen(fb[posy].c_str());
    e.posx = posx;
    e.posy = posy;
    //    std::cout << "DBG> posx=" << int(posx) << " fb[posy].capacity()=" << fb[posy].capacity()
    //	      << std::endl;
    fb[posy].append(str);
    fb[posy].append(" ");
    //    std::cout << "fb[" << int(posy) << "]='" << fb[posy].data() << "'" << std::endl;
    index++;
    
    //    std::cout << "DBG> selectedItem = " << int(selectedItem) << "  str[" << str.length() << "]=" << str.data() << std::endl;

  }
}


template <size_t SW, size_t SL>
void MenuEntries<SW, SL>::print(void) const
{
  for (const auto& s : fb) {
    std::cout << s.data() << std::endl;
  }
  std::cout << std::endl;

   for (const auto& e : entries) {
     std::cout << e.str.data() << " posy=" << int(e.posy) << " posx=" << int(e.posx) << std::endl;
  }
}

template <size_t SW, size_t SL>
void MenuEntries<SW, SL>::next(void)
{
  selectedItem = (selectedItem+1U) % entries.size();
  BaseEntry<SW, SL>::draw();
}

template <size_t SW, size_t SL>
void MenuEntries<SW, SL>::prev(void)
{
  selectedItem = selectedItem==0U ? entries.size() - 1U :
    selectedItem - 1U ;
  BaseEntry<SW, SL>::draw();
}
 

