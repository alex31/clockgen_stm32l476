#include "mainTab.hpp"

void MainTab::enter(void)
{
  fb.write(0,0, "F1  = %d%*c", 10, LCD_WIDTH, ' ');
  fb.write(0,1, "F2  = %d%*c", 20, LCD_WIDTH, ' ');
  fb.write(0,2, "Fin = %d%*c", 100, LCD_WIDTH, ' ');
  fb.write(0,3, "Vlc = %.2f%*c", 4.97f, LCD_WIDTH, ' ');
  print();
}


void MainTab::leave(void) 
{
}


void MainTab::eventCb(const Event& ev) 
{
  switch(ev.getEvent())  {
  case Events::Turn : {
    const uint8_t line = ev.getIndex();
    fb.write(0, line, "F%u=%d          ", line+1, ev.getLoad()).print();
    break;
  }
    
    
  case Events::ShortClick : {
    const uint8_t line = ev.getIndex();
    fb.write(0, line, "F%u=%s          ", line+1, "short").print();
    break;
  }
    
  case Events::LongClick :
    IhmState::push(StateId::FreqShortCut);
    break;

  case Events::DoubleClick :
    IhmState::push(StateId::Info);
    break;
    
  default: break;
  }
}


