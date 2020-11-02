#pragma once
#include "ch.h" 
#include "hal.h" 
#include "hardwareConf.hpp"
#include <array>
#include <string.h>

namespace FRAM {
  bool init(void) ;
  template <typename T>
  bool write(const T& object, const uint16_t memAddr);
  template <typename T>
  bool read(T& object, const uint16_t memAddr);
  void resetI2c(I2CDriver *i2cd);
  constexpr uint8_t slaveNumberBase = 0b1010000;
}




template <typename T>
bool FRAM::write(const T& object, const uint16_t memAddr)
  {
    std::array<uint8_t, sizeof(T)+1> wbuffer;
    const uint8_t slaveNumber = slaveNumberBase | ((memAddr >> 8) & 0x07);
    const uint8_t memAddrLsb = memAddr & 0xFF;
    wbuffer[0] = memAddrLsb;
    memcpy(&wbuffer[1], &object, sizeof(T));
    i2cAcquireBus(&I2C_FRAM);
    msg_t res = i2cMasterTransmitTimeout(&I2C_FRAM, slaveNumber,
					 wbuffer.data(), wbuffer.size(),
					 nullptr, 0, TIME_MS2I(10));
    
    if (res != MSG_OK) {
      FRAM::resetI2c(&I2C_FRAM);
    }

    i2cReleaseBus(&I2C_FRAM);
    return res == MSG_OK;
  }
 

template <typename T>
bool FRAM::read(T& object, const uint16_t memAddr)
  {
    const uint8_t slaveNumber = slaveNumberBase | ((memAddr >> 8) & 0x07);
    const uint8_t memAddrLsb = memAddr & 0xFF;
    msg_t res = i2cMasterTransmitTimeout(&I2C_FRAM, slaveNumber,
					 &memAddrLsb, sizeof(memAddrLsb),
					 reinterpret_cast<uint8_t *>(&object),
					 sizeof(T), TIME_MS2I(10));
    return res == MSG_OK;
  }
 


