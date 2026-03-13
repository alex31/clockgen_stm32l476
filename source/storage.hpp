#pragma once
#include "ch.h"
#include "array"

class Storage
{
public:
  Storage(Storage const &) = delete;
  Storage& operator=(Storage const &) = delete;

  static Storage &instance(void) {
    static Storage inst;
    return inst;
  }

  bool load(void);
  bool store(void);
  static bool hasFailed(void) {return i2cFailAtInit;}

  void setFrequency(const uint8_t idx, const uint32_t f) {
    frequencies[idx] = f;
    store();
  }
  const std::array<uint32_t, 2>& getFrequencies(void) {return frequencies;}
  void setVoltageRef(const float v) {voltageRef = v; store();}
  void incAge(void) {age++; store();}
  void incPowerOn(void) {powerOn++; store();}
  void incI2cFailure(void) {i2cFailure++; store();}
  void incWatchdogReset(void) {watchdogReset++; store();}
  void resetAlert(void) {
    i2cFailure = underVoltageAlert =
      overVoltageAlert = psFailureAlert = 0;
    store();
  }
  void incUnderVoltageAlertX(void) {underVoltageAlert++; storePending = true;}
  void incOverVoltageAlert(void) {overVoltageAlert++; store();}
  void incPsFailureAlertX(void) {psFailureAlert++; storePending = true;}
  void setVolume(const uint8_t v) {volume = v; store();}
  void setSampleIndex(const uint8_t i) {sampleIndex = i; store();}
  void setEnableF2(const bool e) {enableF2 = e;}
  
  float getVoltageRef(void) const {return voltageRef;}
  uint32_t getAge(void) const {return age;}
  uint32_t getPowerOn(void) const {return powerOn;}
  uint32_t getUnderVoltageAlert(void) const {return underVoltageAlert;}
  uint32_t getOverVoltageAlert(void) const {return overVoltageAlert;}
  uint32_t getPsFailureAlert(void) const {return psFailureAlert;}
  uint32_t getI2cFailure(void) const {return i2cFailure;}
  uint32_t getWatchdogReset(void) const {return watchdogReset;}
  uint8_t getVolume(void) const {return volume;}
  uint8_t getSampleIndex(void) const {return sampleIndex;}
  bool	  getEnableF2(void) const {return enableF2;}
  void print(void);

  
private:
  Storage(void);
  ~Storage() {}
  static void periodicStore(void *arg);
  static volatile bool storePending;
  static bool i2cFailAtInit;
  constexpr static uint32_t LEGACY_MAGIC = 0xBEEFC0DE;
  constexpr static uint32_t MAGIC = 0xBEEFC0DF;
  uint32_t magic;
  std::array<uint32_t, 2> frequencies;
  float voltageRef;
  uint32_t age; // seconds
  uint32_t powerOn; // number of occurences
  uint32_t underVoltageAlert;
  uint32_t overVoltageAlert;
  uint32_t psFailureAlert;
  uint32_t i2cFailure;
  uint32_t watchdogReset;
  uint8_t volume;
  uint8_t sampleIndex;
  bool	  enableF2;
};
