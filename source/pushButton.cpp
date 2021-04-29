#include "pushButton.hpp"
#include "event.hpp"
#include <functional>
#include "encoderTimer.hpp"



bool PushButton::init()
{
  if (palReadLine(line) == PAL_LOW) {
    Event ev(Events::ClickAtPowerOn, index);
    chMBPostTimeout(&EVT::mb, ev.getEventAsMsg(), TIME_INFINITE);
  }
  palEnableLineEvent(line, PAL_EVENT_MODE_BOTH_EDGES);
  chVTObjectInit(&vt);
  return true;
}



bool PushButton::loop()
{
  palWaitLineTimeout(line, TIME_INFINITE);

  if (palReadLine(line) == PAL_HIGH)
    proceedUp();
  else
    proceedDown();
  
  return true;
}


void PushButton::proceedUp(void)
{
  chVTReset(&vt);
  chVTSet(&vt, TIME_MS2I(ANTI_REBOUND_INTERVAL_MS),
	  [] (void *arg) {
	    chSysLockFromISR();
	    PushButton *pb = static_cast<PushButton *>(arg);
	    if (palReadLine(pb->line) == PAL_HIGH) {
	      pb->ts =  chVTGetSystemTimeX();
	      if (pb->state == PBState::Down) {
		Event ev(Events::ShortClick, pb->index);
		chMBPostI(&EVT::mb, ev.getEventAsMsg());
	      }
	      pb->state = PBState::Up;
	    }
	    chSysUnlockFromISR();
	  }, this);
}


void PushButton::proceedDown(void)
{
  chVTReset(&vt);

  chVTSet(&vt, TIME_MS2I(ANTI_REBOUND_INTERVAL_MS),
	  [] (void *arg) {
	    chSysLockFromISR();
	    PushButton *pb = static_cast<PushButton *>(arg);
	    if (palReadLine(pb->line) == PAL_LOW) {
	      pb->state = PBState::Down;
	      if (chTimeIsInRangeX(chVTGetSystemTimeX(),
				   pb->ts, 
				   pb->ts + TIME_MS2I(DOUBLE_CLIC_INTERVAL_MS))) {
		Event ev1(Events::Undo, pb->index);
		Event ev2(Events::DoubleClick, pb->index);
		chMBPostI(&EVT::mb, ev1.getEventAsMsg());
		chMBPostI(&EVT::mb, ev2.getEventAsMsg());
		pb->state = PBState::DoubleClick;
	      } else {
		chVTResetI(&pb->vt);
		chVTSetI(&pb->vt, TIME_MS2I(LONG_CLIC_INTERVAL_MS),
			 [] (void *aarg) {
			   chSysLockFromISR();
			   PushButton *ppb = static_cast<PushButton *>(aarg);
			   Event ev(Events::LongClick, ppb->index);
			   chMBPostI(&EVT::mb, ev.getEventAsMsg());
			   ppb->state = PBState::LongDown;
			   chSysUnlockFromISR();
			 }, pb);
	      }
	    }
	    chSysUnlockFromISR();     
	  }, this);
}
	  

uint32_t PushButton::factoryIdx = 0U;
std::array<PushButton*, 2> PushButton::buttons{};

