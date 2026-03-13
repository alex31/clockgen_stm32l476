#include "storage.hpp"
#include "fram.hpp"
#include <array>
#include <cstddef>
#ifndef NOSHELL
#include "stdutil.h"
#endif

namespace {
  struct StorageImageV1 {
    uint32_t magic;
    std::array<uint32_t, 2> frequencies;
    float voltageRef;
    uint32_t age;
    uint32_t powerOn;
    uint32_t underVoltageAlert;
    uint32_t overVoltageAlert;
    uint32_t psFailureAlert;
    uint32_t i2cFailure;
    uint8_t volume;
    uint8_t sampleIndex;
    bool enableF2;
    uint8_t padding;
  };

  struct StorageImageV2 {
    uint32_t magic;
    std::array<uint32_t, 2> frequencies;
    float voltageRef;
    uint32_t age;
    uint32_t powerOn;
    uint32_t underVoltageAlert;
    uint32_t overVoltageAlert;
    uint32_t psFailureAlert;
    uint32_t i2cFailure;
    uint32_t watchdogReset;
    uint8_t volume;
    uint8_t sampleIndex;
    bool enableF2;
    uint8_t padding;
  };

  static_assert(sizeof(StorageImageV1) == 44U, "Unexpected legacy storage size");
  static_assert(sizeof(StorageImageV2) == 48U, "Unexpected storage size");
}

Storage::Storage(void) {
  chThdCreateFromHeap(nullptr, 1024U, "periodicStore", NORMALPRIO,
		      &periodicStore, nullptr);
}

bool Storage::load(void)
{
  StorageImageV2 image = {};
  if (FRAM::read(image, 0) == true) {
    if (image.magic == MAGIC) {
      magic = image.magic;
      frequencies = image.frequencies;
      voltageRef = image.voltageRef;
      age = image.age;
      powerOn = image.powerOn;
      underVoltageAlert = image.underVoltageAlert;
      overVoltageAlert = image.overVoltageAlert;
      psFailureAlert = image.psFailureAlert;
      i2cFailure = image.i2cFailure;
      watchdogReset = image.watchdogReset;
      volume = image.volume;
      sampleIndex = image.sampleIndex;
      enableF2 = image.enableF2;
      return true;
    }
  } else {
    i2cFailAtInit = true;
    return false;
  }

  StorageImageV1 legacy = {};
  if (FRAM::read(legacy, 0) == true && legacy.magic == LEGACY_MAGIC) {
    magic = MAGIC;
    frequencies = legacy.frequencies;
    voltageRef = legacy.voltageRef;
    age = legacy.age;
    powerOn = legacy.powerOn;
    underVoltageAlert = legacy.underVoltageAlert;
    overVoltageAlert = legacy.overVoltageAlert;
    psFailureAlert = legacy.psFailureAlert;
    i2cFailure = legacy.i2cFailure;
    watchdogReset = 0U;
    volume = legacy.volume;
    sampleIndex = legacy.sampleIndex;
    enableF2 = legacy.enableF2;
    store();
    return true;
  }

  magic = MAGIC;
  frequencies = {1U,1U};
  volume = 50U;
  sampleIndex = 0U;
  voltageRef = 5.0f;
  age = 0U;
  powerOn = 0U;
  underVoltageAlert = 0U;
  overVoltageAlert = 0U;
  psFailureAlert = 0U;
  i2cFailure = 0U;
  watchdogReset = 0U;
  enableF2 = false;
  store();
  // DebugTrace("====== READ ========");
  // print();
  // DebugTrace("\n");
  return true;
}

bool Storage::store(void)
{
  const StorageImageV2 image = {
    .magic = magic,
    .frequencies = frequencies,
    .voltageRef = voltageRef,
    .age = age,
    .powerOn = powerOn,
    .underVoltageAlert = underVoltageAlert,
    .overVoltageAlert = overVoltageAlert,
    .psFailureAlert = psFailureAlert,
    .i2cFailure = i2cFailure,
    .watchdogReset = watchdogReset,
    .volume = volume,
    .sampleIndex = sampleIndex,
    .enableF2 = enableF2,
    .padding = 0U,
  };
  //  DebugTrace("====== STORE ========");
  //print();
  //  DebugTrace("\n");
  return (not i2cFailAtInit) and FRAM::write(image, 0);
}

void Storage::print(void)
{
#ifndef NOSHELL
  DebugTrace("magic = 0x%lx", magic);
  DebugTrace("frq = %lu, %lu", frequencies[0], frequencies[1]);
  DebugTrace("voltageRef = %.2f", voltageRef);
  DebugTrace("age = %lu", age);
  DebugTrace("power on = %lu", powerOn);
  DebugTrace("underVoltageAlert = %lu", underVoltageAlert);
  DebugTrace("overVoltageAlert = %lu", overVoltageAlert);
  DebugTrace("psFailureAlert = %lu", psFailureAlert);
  DebugTrace("i2cFailure = %lu", i2cFailure);
  DebugTrace("watchdogReset = %lu", watchdogReset);
  DebugTrace("volume = %u", volume);
  DebugTrace("sampleIndex = %u", sampleIndex);
  DebugTrace("enableF2 = %u", enableF2 ? 1U : 0U);
#endif
}


bool Storage::i2cFailAtInit = false;
volatile bool Storage::storePending = false;

void Storage::periodicStore([[maybe_unused]] void *arg)
{
  while (true) {
    if (storePending) {
      instance().store();
      storePending = false;
    }
    chThdSleepMilliseconds(10);
  }
}
