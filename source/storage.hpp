#pragma once
#include "ch.h"
#include "array"

class Storage
{
public:
  constexpr Storage(void) = default;
  bool load(void);
  bool store(void);
  static bool hasFailed(void) {return failure;}

  void setFrequency(const uint8_t idx, const uint32_t f) {
    frequencies[idx] = f;
    store();
  }
  const std::array<uint32_t, 2>& getFrequencies(void) {return frequencies;}
  void setVoltageRef(const float v) {voltageRef = v; store();}
  void incAge(void) {age++; store();}
  void incPowerOn(void) {powerOn++; store();}
  void incI2cFailure(void) {i2cFailure++; store();}
  void resetAlert(void) {
    i2cFailure = underVoltageAlert =
      overVoltageAlert = psFailureAlert = 0;
    store();
  }
  void incUnderVoltageAlert(void) {underVoltageAlert++; store();}
  void incOverVoltageAlert(void) {overVoltageAlert++; store();}
  void incPsFailureAlert(void) {psFailureAlert++; store();}
  void setVolume(const uint8_t v) {volume = v; store();}
  void setSampleIndex(const uint8_t i) {sampleIndex = i; store();}
  
  float getVoltageRef(void) const {return voltageRef;}
  uint32_t getAge(void) const {return age;}
  uint32_t getPowerOn(void) const {return powerOn;}
  uint32_t getUnderVoltageAlert(void) const {return underVoltageAlert;}
  uint32_t getOverVoltageAlert(void) const {return overVoltageAlert;}
  uint32_t getPsFailureAlert(void) const {return psFailureAlert;}
  uint32_t getI2cFailure(void) const {return i2cFailure;}
  uint8_t getVolume(void) const {return volume;}
  uint8_t getSampleIndex(void) const {return sampleIndex;}
  void print(void);

  
private:
  static Storage* singletonCheck;
  static bool failure;
  constexpr static uint32_t MAGIC = 0xDEADBEEF;
  uint32_t magic;
  std::array<uint32_t, 2> frequencies;
  float voltageRef;
  uint32_t age; // seconds
  uint32_t powerOn; // number of occurences
  uint32_t underVoltageAlert;
  uint32_t overVoltageAlert;
  uint32_t psFailureAlert;
  uint32_t i2cFailure;
  uint8_t volume;
  uint8_t sampleIndex;
};
