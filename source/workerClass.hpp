#pragma once
#include <ch.h>
#include <hal.h>



template<typename T>
class WorkerThread
{
public:
  enum ExitCode {ERROR_IN_INIT=-10, ERROR_IN_LOOP=-11};

  WorkerThread(const char *m_name, const size_t m_size,
	       const tprio_t m_prio) : name(m_name), stackSize(m_size), prio(m_prio) {};
  bool run(sysinterval_t time);
  WorkerThread& terminate();
  WorkerThread& join();
  virtual ~WorkerThread() {terminate().join();};
  
protected:
  static void threadFunc(void *o);
  // this is called in origin thread context
  virtual bool init(void) = 0;
  // this is called in newly created thread context
  virtual bool initInThreadContext(void) {return true;};
  virtual bool loop(void) = 0;

private:
  const char *name;
  const size_t  stackSize;
  const tprio_t prio;
  sysinterval_t timeInLoop;
  thread_t *handle = nullptr;
};


template<typename T>
class WorkerThreadSingleton : public WorkerThread<T>
{
public:
  WorkerThreadSingleton() = delete;
  WorkerThreadSingleton(WorkerThreadSingleton const &) = delete;
  WorkerThreadSingleton& operator=(WorkerThreadSingleton const &) = delete;
  
  static T &instance(const tprio_t m_prio) {
    static T inst(m_prio);
    return inst;
  }
protected:
  WorkerThreadSingleton(const char *m_name, const size_t m_size,
		     const tprio_t m_prio) : WorkerThread<T>(m_name, m_size, m_prio) {};

  ~WorkerThreadSingleton() {}
};



/*
  thread_t * 	chThdCreateFromHeap (memory_heap_t *heapp, size_t size, const char *name, 
		tprio_t prio, tfunc_t pf, void *arg)
 */
template<typename T>
bool WorkerThread<T>::run(sysinterval_t m_timeInLoop)
{
 if (init() == false)
    return false;

 timeInLoop = m_timeInLoop;
  //  handle = chThdCreateStatic(ws, sizeof(ws), prio, threadFunc, this);
  handle = chThdCreateFromHeap(nullptr, stackSize, name, prio, threadFunc, this);
  return true;
}

template<typename T>
WorkerThread<T>& WorkerThread<T>::join(void)
{
  // this will force that only one process can be run
  // in the same time
  if (handle != nullptr) {
    chThdWait(handle);
    handle = nullptr;
  }
  return *this;
}

template<typename T>
WorkerThread<T>& WorkerThread<T>::terminate(void)
{
  // this will force that only one process can be run
  // in the same time, not really a singleton but close. 
  if (handle != nullptr) 
    chThdTerminate(handle);
  
  return *this;
  
}


template<typename T>
void WorkerThread<T>::threadFunc(void *o) {
  T * const self = static_cast<T*>(o);
  if (self->initInThreadContext() == false)
    chThdExit(ERROR_IN_INIT);


  while (!chThdShouldTerminateX()) {
    const systime_t now = chVTGetSystemTimeX();
    const systime_t then = chTimeAddX(now, self->timeInLoop);
    if (self->loop() == false)
      break;
    if (self->timeInLoop)
      chThdSleepUntilWindowed(now, then);
  }
  chThdExit(ERROR_IN_LOOP);
}

