#include "oneColTab.hpp"

void OneColTab::enter(void)
{
  invoque(Enter);
  widget->draw();
  widget->invoque();
}


void OneColTab::leave(void) 
{
  invoque(Leave);
}


void OneColTab::eventCb(const Event& ev) 
{
  switch(ev.getEvent())  {
  case Events::Turn : {
    if (ev.getLoad() > 0)
      widget->next();
    else
      widget->prev();
    break;
  }
    
  case Events::ShortClick : {
    IhmState::pop();
    break;
  }

  case Events::Periodic : {
    widget->refresh();
    break;
  }
    
  default: break;
  }
}


