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
      if (chMBFetchTimeout(&mb, (msg_t *) &ev, TIME_MS2I(200)) == MSG_OK)
	cb(ev);
      else
	cb({Events::Periodic, 0, 0});
    }
  }
}


void Event::init(callback_t cb)
{
  chThdCreateStatic(EVT::waEventSerializer, sizeof(EVT::waEventSerializer),
		    NORMALPRIO+2, &EVT::eventSerializer, (void *) cb);
}
