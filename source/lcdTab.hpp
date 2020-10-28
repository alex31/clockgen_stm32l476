#pragma once
#include "ihmState.hpp"
#include "frameBuffer.hpp"
#include "hardwareConf.hpp"
#include "event.hpp"

class LcdTab : public IhmState {
public:
  enum  Action{Enter, Leave};
  using callback_t = std::function<void(void)>;
  LcdTab (const StateId stateId) : IhmState(stateId) {}
  virtual void enter(void) override = 0 ;
  virtual void leave(void) override = 0 ;
  virtual void eventCb(const Event& ev) = 0;
  void print(void) {fb.print();}
  void bind(const Action action, callback_t cb) {
    actionCB[action] = cb;
  }
  void invoque(const Action action) {
    if (actionCB[action] != nullptr) {
      actionCB[action]();
    }
  }
  static void propagate(const Event& ev);
  static LcdTab* push(const StateId active);
  
  
protected:
  static FrameBuffer<LCD_WIDTH, LCD_HEIGHT> fb;
  callback_t actionCB[2] = {nullptr, nullptr};
};
