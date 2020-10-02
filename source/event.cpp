#include "event.hpp"


namespace EVT {
  msg_t msgBuf[MB_LEN];
  MAILBOX_DECL(mb, msgBuf, MB_LEN);
  THD_WORKING_AREA(waEventSerializer, threadStackSize);








  void eventSerializer(void *arg)
  {
    chRegSetThreadName("evt serializer");
    Event ev;
    callback_t cb = (callback_t) arg;
    while (true) {
      chMBFetchTimeout(&mb, (msg_t *) &ev, TIME_INFINITE);
      cb(ev);
    }
  }
}


void Event::init(callback_t cb)
{
  chThdCreateStatic(EVT::waEventSerializer, sizeof(EVT::waEventSerializer),
		    NORMALPRIO+2, &EVT::eventSerializer, (void *) cb);
}
