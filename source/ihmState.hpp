#pragma once
#include <type_traits>
#include <array>
#include "etl/string.h"
#include "etl/stack.h"


template <typename ENM>
constexpr auto operator+(ENM e) {return static_cast<std::underlying_type_t<ENM>>(e);}
enum class StateId {Freq, FreqShortCut, Info, Readme, AudioVol, AudioChoice, EndOfList};


class IhmState
{
public:
  IhmState(const StateId& stateId);
  virtual void enter(void) = 0;
  virtual void leave(void) = 0;
  static IhmState* push(const StateId& active);
  static IhmState* pop(void);
  static void transition(IhmState* from, IhmState* to);

private:
  static std::array<IhmState *, +StateId::EndOfList> stateArray;
  static etl::stack<IhmState*, 8>  stack;
};
