#include "twoColsTab.hpp"



void TwoColsTab::enter(void)
{
  left.draw();
  right->draw();
}


void TwoColsTab::leave(void) 
{
}


void TwoColsTab::eventCb(const Event& ev) 
{
  switch(ev.getEvent())  {
  case Events::Turn : {
    if(ev.getIndex() == 0) {
      if (ev.getLoad() > 0)
	left.next();
      else
	left.prev();
	} else {
      if (ev.getLoad() > 0)
	right->next();
      else
	right->prev();
    }
    break;
  }
    
    
  case Events::ShortClick : {
    IhmState::pop();
  }
    
    
  default: break;
  }
}


