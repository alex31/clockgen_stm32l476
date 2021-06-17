#include <ch.h>
#include <hal.h>
#include "resetTTL.hpp"

namespace {
  virtual_timer_t vt;
}

void releaseResetAfter(const sysinterval_t after)
{
  chVTObjectInit(&vt);
  chVTSet(&vt, after,
	  [] ([[maybe_unused]] ch_virtual_timer *vtl, [[maybe_unused]] void *arg) {
	    palSetLine(LINE_TTL_RESET_OUT);
	  }
	  , nullptr);
}
