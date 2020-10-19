#include <cstdlib>
static constexpr size_t LCD_WIDTH = 20U;
static constexpr size_t LCD_HEIGHT = 4U;

#include "menuEntries.hpp"
#include <iostream>
#include <algorithm>

// g++-10 -Wall -std=c++20 -I../../../../../etl/include/ -I.  menuEntries.cpp
// g++-9 -Wall -std=c++2a -I../../../../../etl/include/ -I.  menuEntries.cpp


FrameBuffer<MenuEntries::W, MenuEntries::L>::FbView MenuEntries::getView(const size_t index)
{
  int bgin = 0;
  if (index < entries.size()) {
    bgin = int(entries[index].line) - 1;
    bgin = std::clamp(bgin, 0, int(LCD_HEIGHT) - 4);
  }
  std::cout << "DBG> bgin=" << bgin << std::endl;
  return fb.getView(bgin, 4);
}


MenuEntries::MenuEntries(std::initializer_list<Entry> il)
{
  for (const auto& e : il)
    addEntry(e);
}


bool MenuEntries::addEntry(const Entry& e)
{
  if (entries.full())
    return false;
  
  entries.push_back({
		     .value = e.value,
		     .str = (FixedStr(" ") += e.str) += " ",
		     .line = e.line,
		     .posx = e.posx
    });
  return true;
}

void MenuEntries::fill(const uint8_t margin, const etl::string_view sep)
{
  uint8_t line=0U;
  for (auto& s : fb) {
    s.insert(s.begin(), margin, ' ');
    if (sep.length())
      s.append(sep.data());
  }

  for (size_t index=0; auto& e : entries) {
    std::cout << "DBG> str[" << e.str.length() << "]=" << e.str.data() << std::endl;
    auto str = e.str;
    if (index == selectedItem) {
      str.replace(str.begin(),str.begin()+1, "("); 
      str.replace(str.end()-1, str.end(), ")"); 
    }
    if (str.length() >= fb[line].available()) {
      line++;
      if (line >= fb.getHeight())
	break;
    }
    const uint8_t posx = fb[line].length();
    e.posx = posx;
    e.line = line;
    //    std::cout << "DBG> posx=" << int(posx) << " fb[line].capacity()=" << fb[line].capacity()
    //	      << std::endl;
    fb[line].append(" ");
    fb[line].append(str);
    //    std::cout << "fb[" << int(line) << "]='" << fb[line].data() << "'" << std::endl;
    index++;
  }
}


void MenuEntries::print(void) const
{
  for (const auto& s : fb) {
    std::cout << s.data() << std::endl;
  }
  std::cout << std::endl;

   for (const auto& e : entries) {
     std::cout << e.str.data() << " line=" << int(e.line) << " posx=" << int(e.posx) << std::endl;
  }
}

void MenuEntries::print(FrameBuffer<W, L>::FbView fbv)
{
  for (const auto& s : fbv) {
    std::cout << s.data() << std::endl;
  }
  std::cout << std::endl;
}


/* ********* */

void NumericEntry::print(void) const
{
  for (const auto& s : fb) {
    std::cout << s.data() << std::endl;
  }
}

void NumericEntry::fill(const uint8_t margin, const etl::string_view sep)
{
  for (auto& s : fb) {
    s.insert(s.begin(), margin, ' ');
    if (sep.length())
      s.append(sep.data());
  }
  // faut pouvoir imprimer dans un framebuffer

}

FrameBuffer<NumericEntry::W, NumericEntry::L>::FbView NumericEntry::getView()
{
  return fb.getView(0, 1);
}




int main(void)
{
  MenuEntries me({{1, "1_Hz"},
		  {20, "20_Hz"},
		  {300, "300_hz"},
		  {400, "400_hz"},
		  {2400, "2.4_Khz"},
		  {5000, "5_Khz"},
		  {8000, "8_Khz"},
		  {10000, "10_Khz"},
		  {19200, "19.2_Khz"},
		  {36400, "36.4_Khz"}
    });
  me.fill(5, "| ");
  me.print();
  std::cout << "#################" << std::endl;
  MenuEntries::print(me.getView(4));

  std::cout << "#################" << std::endl;
  FrameBuffer<5,5> fb55;
  FrameBuffer<2,4> fb22;
  fb55[0] = fb55[1] = fb55[2] = fb55[3] = fb55[4] = "12345";
  fb22[0] = fb22[1] = fb22[2] = fb22[3] = "AB";
  fb55.copyRect(fb22.getView(0,2), 2, 2);

  for (const auto& str : fb55)
    std::cout << str.data() << std::endl;

  return 0;
}
