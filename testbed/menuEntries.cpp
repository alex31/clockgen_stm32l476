#include "menuEntries.hpp"
#include <iostream>
#include <algorithm>

MenuEntries::MenuEntries(std::initializer_list<Entry> il)
{
  for (const auto& e : il)
    addEntry(e);
}


bool MenuEntries::addEntry(const Entry& e)
{
  if (entries.full())
    return false;

  entries.push_back(e);
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

  for (auto& e : entries) {
    //    std::cout << "DBG> str[" << e.str.length() << "]=" << e.str.data() << std::endl;
    if (e.str.length() >= fb[line].available()) {
      line++;
      if (line >= fb.size())
	break;
    }
    const uint8_t posx = fb[line].length();
    e.posx = posx;
    e.line = line;
    //    std::cout << "DBG> posx=" << int(posx) << " fb[line].capacity()=" << fb[line].capacity()
    //	      << std::endl;
    fb[line].append(" ");
    fb[line].append(e.str);
    //    std::cout << "fb[" << int(line) << "]='" << fb[line].data() << "'" << std::endl;
  }
}

FrameBufferView MenuEntries::getView(const size_t index)
{
  int bgin = 0;
  if (index < entries.size()) {
    bgin = int(entries[index].line) - 1;
    bgin = std::clamp(bgin, 0, int(DP::virtualHeight) - 4);
  }
  std::cout << "DBG> bgin=" << bgin << std::endl;
  return FrameBufferView{&fb[bgin], 4};
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

void MenuEntries::print(FrameBufferView fbv)
{
  for (const auto& s : fbv) {
    std::cout << s.data() << std::endl;
  }
  std::cout << std::endl;
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



  FrameeBuffer<20, 4> fb2;

  
  return 0;
}
