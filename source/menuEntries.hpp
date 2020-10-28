#pragma once

#include "hardwareConf.hpp"
#include <array>
#include <cstdint>
#include "etl/string.h"
#include "etl/vector.h"
#include "etl/string_view.h"
#include "etl/span.h"
#include <functional>
#include "frameBuffer.hpp"
#include "ihmState.hpp"
#include "lcdTab.hpp"


#ifdef SIM_PC
#include <iostream>
#else
#include "lcdDisplay.hpp"
#endif


using FixedStr = etl::string<20>;

struct Entry {
  int value;
  FixedStr str;
  StateId nextState=StateId::None;
  uint8_t posx=0;
  uint8_t posy=0;
};


class BaseWidget {
public:
  using callback_t = std::function<void(int32_t)>;

  virtual void next(void) = 0;
  virtual void prev(void) = 0;
  virtual void draw(void) = 0;
  virtual void bind(callback_t _cb) = 0;
  virtual void invoque(void) = 0;
  virtual void refresh(void) =0;
  virtual void goSubMenu(void) =0;
  virtual const FixedStr& getName(void) =0;
  virtual void  setParentFb(FrameBuffer<LCD_WIDTH, LCD_HEIGHT> *fb) = 0;
};



template <size_t SW, size_t SL>
class BaseEntry : public BaseWidget {
public:
  BaseEntry(const FixedStr& _name,
	    FrameBuffer<LCD_WIDTH, LCD_HEIGHT> *fb,
	    uint8_t _anchorx, uint8_t _anchory) : parentFb(fb),
						  anchorx(_anchorx),
						  anchory(_anchory),
						  name(_name) {}
  virtual void next(void) override = 0;
  virtual void prev(void) override = 0;
  virtual void fill(const uint8_t margin = 0U, const etl::string_view sep = "") = 0;
  void	       set(const int32_t v) {val = v; this->invoque(); draw();}
  virtual int32_t     get(void) const {return val;}
  virtual FrameBuffer<SW, SL>::FbView getView(void) = 0;
  void    setParentFb(FrameBuffer<LCD_WIDTH, LCD_HEIGHT> *fb) override {parentFb =fb;}
  virtual void draw(void) override;
  virtual void refresh(void) override {};
  virtual void goSubMenu(void) override {};
  void bind(callback_t _cb) override {cb = _cb;}
  void invoque(void) override {if (cb) cb(get());}
  const FixedStr& getName(void) override {return name;}
protected:
  
  FrameBuffer<LCD_WIDTH, LCD_HEIGHT>* parentFb = nullptr;
  const uint8_t anchorx=0;
  const uint8_t anchory=0;
  int32_t      val=0;
  callback_t cb = nullptr;
  FixedStr name;
};

template <size_t SW, size_t SL>
class MenuEntries : public BaseEntry<SW, SL> {

  
public:
  MenuEntries(const FixedStr& name,
	      FrameBuffer<LCD_WIDTH, LCD_HEIGHT> *fb,
	      uint8_t _anchorx, uint8_t _anchory,
	      std::initializer_list<Entry> il);
  MenuEntries(const FixedStr& name,
	      uint8_t _anchorx, uint8_t _anchory,
	      std::initializer_list<Entry> il);
  bool addEntry(const Entry& e);
  const Entry& operator[](const size_t index) const {return entries[index];}
  void fill(const uint8_t margin = 0U, const etl::string_view sep = "") override;
  void next(void) override;
  void prev(void) override;
  virtual void goSubMenu(void) override;

  FrameBuffer<SW, SL>::FbView getView(void) override;
  virtual int32_t     get(void) const override {return entries[this->val].value;}
private:

  static constexpr size_t MaxEntries = 20;
  etl::vector<Entry, MaxEntries> entries{};
  int32_t &selectedItem = BaseEntry<SW, SL>::val;
  FrameBuffer<SW, SL> fb{};
};

template <size_t SW>
class NumericEntry : public BaseEntry<SW, LCD_HEIGHT> {
  
public:
  NumericEntry(const FixedStr& name,
	       FrameBuffer<LCD_WIDTH, LCD_HEIGHT> *fb,
	       uint8_t anchorx, uint8_t anchory,
	       const int32_t _val, const int32_t _inc,
	       const std::pair<int32_t, int32_t>& _interval) :
    BaseEntry<SW, LCD_HEIGHT>(name, fb, anchorx, anchory),
	       inc(_inc),
    interval(_interval) {this->val = _val;}
  NumericEntry(const FixedStr& name,
	       uint8_t anchorx, uint8_t anchory,
	       const int32_t _val, const int32_t _inc,
	       const std::pair<int32_t, int32_t>& _interval) :
    BaseEntry<SW, LCD_HEIGHT>(name, nullptr, anchorx, anchory),
	       inc(_inc),
	       interval(_interval) {this->val = _val;};
  void fill(const uint8_t margin = 0U, const etl::string_view sep = "") override;

  void next(void) override {this->val += inc;
    this->val=std::clamp(this->val, interval.first, interval.second);
    this->invoque();
    this->draw();}
  void prev(void) override {this->val -=inc;
    this->val=std::clamp(this->val, interval.first, interval.second);
    this->invoque();
    this->draw();}
  FrameBuffer<SW, LCD_HEIGHT>::FbView getView(void) override;


private:
  int32_t inc = 0U;
  const std::pair<int32_t, int32_t> interval;
  FrameBuffer<SW, LCD_HEIGHT> fb{};
};





template <size_t SH>
class ScrollText : public BaseEntry<LCD_WIDTH, SH> {
  
public:
  ScrollText(const FixedStr& name,
	      FrameBuffer<LCD_WIDTH, LCD_HEIGHT> *fb,
	      FrameBuffer<LCD_WIDTH, SH> &&_content) :
    BaseEntry<LCD_WIDTH, SH>(name, fb, 0, 0),
    content(std::move(_content)) {};
  ScrollText(const FixedStr& name,
	      FrameBuffer<LCD_WIDTH, SH> &&_content)
	      :
    BaseEntry<LCD_WIDTH, SH>(name, nullptr, 0, 0),
    content(std::move(_content)) {};
  ScrollText(const FixedStr& name,
	      FrameBuffer<LCD_WIDTH, LCD_HEIGHT> *fb,
	     std::function<void(FrameBuffer<LCD_WIDTH, SH> &fb)> _drawFn) :
    BaseEntry<LCD_WIDTH, SH>(name, fb, 0, 0),
    drawFn(_drawFn) {};
  ScrollText(const FixedStr& name,
	     std::function<void(FrameBuffer<LCD_WIDTH, SH> &fb)> _drawFn) :
    BaseEntry<LCD_WIDTH, SH>(name, nullptr, 0, 0),
    drawFn(_drawFn) {};
  void fill([[maybe_unused]] const uint8_t margin = 0U,
	    [[maybe_unused]] const etl::string_view sep = "") {};
  void draw(void) override;
  void refresh(void) override{draw();}
  void next(void) override {this->val = std::clamp(this->val+1UL,
						   0UL, SH-LCD_HEIGHT-0UL);
    this->draw();}
  void prev(void) override {if (this->val != 0) this->val--;
    this->draw();}
  FrameBuffer<LCD_WIDTH, SH>::FbView getView(void) override;


private:
  FrameBuffer<LCD_WIDTH, SH> content;
  std::function<void(FrameBuffer<LCD_WIDTH, SH> &fb)> drawFn = nullptr;
};



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
  parentFb->copyRect(getView(), anchorx, anchory);
  parentFb->print();
}

template <size_t SW, size_t SL>
FrameBuffer<SW, SL>::FbView MenuEntries<SW, SL>::getView(void)
{
  int bgin = 0;
  if (selectedItem < int(entries.size())) {
    bgin = int(entries[selectedItem].posy) - 1;
    bgin = std::clamp(bgin, 0, std::max(entries.back().posy - (int(LCD_HEIGHT) - 1), 0));
  }
  //  std::cout << "DBG> LAST (" << entries.back().str.c_str() << ") y=" << int(entries.back().posy) << std::endl;
  //  std::cout << "DBG> bgin=" << bgin << std::endl;
  return fb.getView(bgin, LCD_HEIGHT);
}

template <size_t SW, size_t SL>
MenuEntries<SW, SL>::MenuEntries(const FixedStr& name,
				 FrameBuffer<LCD_WIDTH, LCD_HEIGHT> *fb,
				 uint8_t anchorx, uint8_t anchory,
				 std::initializer_list<Entry> il) :
  BaseEntry<SW, SL>(name, fb, anchorx, anchory)
{
  for (const auto& e : il)
    addEntry(e);
}

template <size_t SW, size_t SL>
MenuEntries<SW, SL>::MenuEntries(const FixedStr& name,
				 uint8_t anchorx, uint8_t anchory,
				 std::initializer_list<Entry> il) :
  BaseEntry<SW, SL>(name, nullptr, anchorx, anchory)
{
  for (const auto& e : il)
    addEntry(e);
}


template <size_t SW, size_t SL>
bool MenuEntries<SW, SL>::addEntry(const Entry& e)
{
  if (entries.full())
    return false;
  
  entries.push_back({
		     .value = e.value,
		     .str = (FixedStr(" ") += e.str) += " ",
		     .nextState = e.nextState,
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

  for (int index=0; auto& e : entries) {
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
void MenuEntries<SW, SL>::next(void)
{
  selectedItem = (selectedItem+1U) % entries.size();
  this->invoque();
  this->draw();
}

template <size_t SW, size_t SL>
void MenuEntries<SW, SL>::prev(void)
{
  selectedItem = selectedItem==0U ? entries.size() - 1U :
    selectedItem - 1U ;
  this->invoque();
  this->draw();
}
 


template <size_t SW>
void NumericEntry<SW>::fill(const uint8_t margin, const etl::string_view sep)
{
  for (auto& s : fb) {
    s.clear();
    s.insert(s.begin(), margin, ' ');
    if (sep.length())
      s.append(sep.data());
  }
  fb.append(0, "<%d>%*c", interval.first, LCD_WIDTH, ' ');
  fb.append(1, "[%d]%*c", this->val, LCD_WIDTH, ' ');
  fb.append(2, "<%d>%*c", interval.second, LCD_WIDTH, ' ');
  fb.append(3, "%*c", LCD_WIDTH, ' ');
}

template <size_t SW>
FrameBuffer<SW, LCD_HEIGHT>::FbView NumericEntry<SW>::getView(void)
{
  return fb.getView(0, LCD_HEIGHT);
}


template <size_t SH>
FrameBuffer<LCD_WIDTH, SH>::FbView ScrollText<SH>::getView(void)
{
  return content.getView(this->val, LCD_HEIGHT);
}

template <size_t SH>
void ScrollText<SH>::draw(void)
{
  if (drawFn != nullptr)
    drawFn(content);
  BaseEntry<LCD_WIDTH, SH>::draw();
}

template <size_t SW, size_t SL>
void MenuEntries<SW, SL>::goSubMenu(void) {
  if (auto &s = entries[this->val].nextState;	s != StateId::None) {
    LcdTab::push(s);
  }
}
