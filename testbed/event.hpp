#pragma once
#include <cstdlib>
#include <cstdint>

enum class Events : uint8_t {None, Turn, ShortClick, LongClick, DoubleClick,
			     SetFreq, Periodic}; 
class Event;
using callback_t = void(*) (const Event&);

using msg_t = uint32_t;

class Event {
public:
  Event() :
    event(Events::None), idx(0), load(0) {}
  Event(const Events _event, const int _idx, const int16_t _load = 0) :
    event(_event), idx(_idx), load(_load) {}
  int  getLoad(void) const {return load;}
  int  getIndex(void) const {return idx;}
  Events getEvent(void) const {return event;}
  msg_t  getEventAsMsg(void) const {return msg;}
  static void init(callback_t cb);

private:
  union {
    struct {
      const Events  event;
      const uint8_t idx:4;
      int32_t       load:20;
    };
    msg_t msg;
  };
};

static_assert(sizeof(Event) == 4);
