#include "ihmState.hpp"
#include "ch.h"

IhmState::IhmState(const StateId stateId)
{
  if (stateId != StateId::EndOfList)
    stateArray[+stateId] = this;
}

IhmState*   IhmState::push(const StateId activeId)
{
  auto active = stateArray[+activeId];
  auto last = stack.top();
  if (not stack.full()) {
    stack.push(active);
    transition(last, active);
  }
  return last;
}

IhmState*   IhmState::pop(void)
{
  if (not stack.empty()) {
    auto last = stack.top();
    stack.pop();
    transition(last, stack.top());
    return last;
  } else {
    return nullptr;
  }
}

void   IhmState::transition(IhmState* from, IhmState* to)
{
  if (from != nullptr) {
    from->leave();
  }
  to->enter();
}



std::array<IhmState *, +StateId::EndOfList> IhmState::stateArray{};
etl::stack<IhmState*, 8> IhmState::stack{};

