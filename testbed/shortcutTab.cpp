#include "shortcutTab.hpp"

void ShortcutTab::enter(void)
{
  me.set(0);
}


void ShortcutTab::leave(void) 
{
}


void ShortcutTab::eventCb(const Event& ev) 
{
  switch(ev.getEvent())  {
  case Events::Turn : {
    if (ev.getLoad() > 0)
      me.next();
    else
      me.prev();

    // setFreq(me.get()); (en fonction de l'id)
    break;
  }
    
    
  case Events::ShortClick : {
    IhmState::pop();
  }
    
  default: break;
  }
}


