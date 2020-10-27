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
    print();
    frequencies[idx] = f;
    store();
  }
  const std::array<uint32_t, 2>& getFrequencies(void) {return frequencies;}
  void setVoltageRef(const float v) {voltageRef = v; store();}
  void incAge(void) {age++; store();}
  void setVoltageAlert(const uint32_t a) {voltageAlert = a; store();}
  void setVolume(const uint8_t v) {volume = v; store();}
  void setSampleIndex(const uint8_t i) {sampleIndex = i; store();}
  
  float getVoltageRef(void) const {return voltageRef;}
  uint32_t getAge(void) const {return age;}
  uint32_t getVoltageAlert(void) const {return voltageAlert;}
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
  uint32_t voltageAlert;
  uint8_t volume;
  uint8_t sampleIndex;
};
