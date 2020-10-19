#include "menuEntries.hpp"
#include "hardwareConf.hpp"
#include <iostream>
#include <algorithm>



/* ********* */
#ifdef PLUS_TARD
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
#endif
