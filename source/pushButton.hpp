#pragma once
#include "workerClass.hpp"
#include "hardwareConf.hpp"
#include "encoderTimer.hpp"
#include "event.hpp"
#include <functional>

namespace PB {
  constexpr size_t threadStackSize = 1024U;
  uint32_t factoryIdx = 0U;
}

enum class PBState {Up, Down, LongDown, DoubleClick};

template<typename U>
class PushButton : public WorkerThread<PB::threadStackSize, PushButton<U>> {
public:
  PushButton(const tprio_t m_prio, const ioline_t _line) :
    WorkerThread<PB::threadStackSize, PushButton>("pushButton", m_prio),
    line(_line)
  {index = PB::factoryIdx++;
    
  };
  //  void setCallback(std::function<void(const Event&)> _cb) {

private:
  friend WorkerThread<PB::threadStackSize, PushButton>;
  bool init(void) final;
  bool loop(void) final;
  void proceedUp(void);
  void proceedDown(void);
  
  ioline_t line;
  //  std::function<void(Event&)> cb;
  uint32_t index;
  virtual_timer_t vt;
  PBState state = PBState::Up;
  systime_t ts = chVTGetSystemTimeX();
};

template<typename U>
bool PushButton<U>::init()
{
  palEnableLineEvent(line, PAL_EVENT_MODE_BOTH_EDGES);
  chVTObjectInit(&vt);
  return true;
}


template<typename U>
bool PushButton<U>::loop()
{
  palWaitLineTimeout(line, TIME_INFINITE);

  if (palReadLine(line) == PAL_HIGH)
    proceedUp();
  else
    proceedDown();
  
  return true;
}

template<typename U>
void PushButton<U>::proceedUp(void)
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

template<typename U>
void PushButton<U>::proceedDown(void)
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
		Event ev(Events::DoubleClick, pb->index);
		chMBPostI(&EVT::mb, ev.getEventAsMsg());
		pb->state = PBState::DoubleClick;
	      } else {
		chVTResetI(&pb->vt);
		chVTSetI(&pb->vt, TIME_MS2I(LONG_CLIC_INTERVAL_MS),
			 [] ([[maybe_unused]] void *arg) {
			   chSysLockFromISR();
			   PushButton *pb = static_cast<PushButton *>(arg);
			   Event ev(Events::LongClick, pb->index);
			   chMBPostI(&EVT::mb, ev.getEventAsMsg());
			   pb->state = PBState::LongDown;
			   chSysUnlockFromISR();
			 }, pb);
	      }
	    }
	    chSysUnlockFromISR();     
	  }, this);
}
	  



