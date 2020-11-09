#pragma once
#include "ch.h"
#include "hal.h"

enum class Events : uint8_t {None, Turn, TurnPress, ShortClick, LongClick,
			     ClickAtPowerOn, DoubleClick,
			     SetFreq, Periodic, UnderVoltage,
			     OverVoltage, Undo}; 
class Event;
using callback_t = void(*) (const Event&);

namespace EVT {
  constexpr size_t threadStackSize = 2048U;
  constexpr size_t MB_LEN = 8U;
  extern msg_t msgBuf[MB_LEN];
  extern mailbox_t mb;
  extern THD_WORKING_AREA(waEventSerializer, threadStackSize);
  void eventSerializer(void *arg);
}

class Event {
public:
  Event() :
    event(Events::None), idx(0), load(0) {}
  Event(const Events _event, const int _idx, const int32_t _load = 0) :
    event(_event), idx(_idx), load(_load) {}
  Event & set (const Events _event,
	       const uint8_t _idx, const int32_t _load = 0) {
    event = _event;
    idx = _idx;
    load = _load;
    return *this;
  }
  int  getLoad(void) const {return load;}
  int  getIndex(void) const {return idx;}
  Events getEvent(void) const {return event;}
  msg_t  getEventAsMsg(void) const {return msg;}
  void print(const char*) const;
  static void init(callback_t cb);

private:
  union {
    struct {
      Events	event;
      uint8_t	idx:3;
      int32_t   load:21;
    };
    msg_t msg;
  };
};

static_assert(sizeof(Event) == 4);
