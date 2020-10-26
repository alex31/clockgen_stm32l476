#include "oneColTab.hpp"

void OneColTab::enter(void)
{
  widget->draw();
}


void OneColTab::leave(void) 
{
}


void OneColTab::eventCb(const Event& ev) 
{
  switch(ev.getEvent())  {
  case Events::Turn : {
    if (ev.getLoad() > 0)
      widget->next();
    else
      widget->prev();

    // setFreq(widget->get()); (en fonction de l'id)
    // via la callback du widget
    break;
  }
    
    
  case Events::ShortClick : {
    IhmState::pop();
  }
    
  default: break;
  }
}


