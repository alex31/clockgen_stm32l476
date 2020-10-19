#include "menuEntries.hpp"
#include <algorithm>

namespace {
  constexpr size_t virtualHeight = 16U;
}

FrameBuffer<MenuEntries::W, MenuEntries::L>::FbView MenuEntries::getView(const size_t index)
{
  int bgin = 0;
  if (index < entries.size()) {
    bgin = int(entries[index].line) - 1;
    bgin = std::clamp(bgin, 0, int(virtualHeight) - 4);
  }
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
    if (e.str.length() >= fb[line].available()) {
      line++;
      if (line >= fb.getHeight())
	break;
    }
    const uint8_t posx = fb[line].length();
    e.posx = posx;
    e.line = line;
    fb[line].append(" ");
    fb[line].append(e.str);
  }
}

