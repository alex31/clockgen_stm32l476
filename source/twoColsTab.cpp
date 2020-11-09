#include "twoColsTab.hpp"

TwoColsTab::TwoColsTab (const StateId stateId, std::initializer_list<BaseWidget *> il) :
  LcdTab(stateId)
{
  for (const auto& e : il)
      rights.push_back(e);
    
  right = rights[0];
  
  for (size_t i=0; i< rights.size(); i++) {
    rights[i]->setParentFb(&fb);
    left.addEntry(Entry{int(i), rights[i]->getName()});
  }
  left.bind([this] (int32_t val) {
	      right =  rights[val];
	      right->draw();
	    });
}


void TwoColsTab::enter(void)
{
  left.draw();
  right->draw();
  invoque(Enter);
}


void TwoColsTab::leave(void) 
{
  invoque(Leave);
}


void TwoColsTab::eventCb(const Event& ev) 
{
  switch(ev.getEvent())  {
  case Events::Turn:
  case Events::TurnPress : {
    if(ev.getIndex() == 0) {
      if (ev.getLoad() > 0)
	left.next();
      else
	left.prev();
	} else {
      if (ev.getLoad() > 0)
	for(int i=0; i< ev.getLoad(); i++)
	  right->next();
      else
	for(int i=0; i> ev.getLoad(); i--)
	  right->prev();
    }
    break;
  }
    
    
  case Events::ShortClick : {
    IhmState::pop();
    break;
  }

  case Events::LongClick : {
    right->goSubMenu();
    break;
  }
    
    
  default: break;
  }
}


