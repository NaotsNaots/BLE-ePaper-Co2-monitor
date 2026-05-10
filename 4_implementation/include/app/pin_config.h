#pragma once

#include <Arduino.h>

#ifdef TARGET_SEEED_XIAO_NRF52840_SENSE
#include <variant.h>
#endif

namespace pin {

constexpr uint8_t kEpdCs = D1;
constexpr uint8_t kEpdDc = D3;
constexpr uint8_t kEpdRst = D6;
constexpr uint8_t kEpdBusy = D7;

#if defined(TARGET_SEEED_XIAO_NRF52840_SENSE) && defined(PIN_LSM6DS3TR_C_INT1)
constexpr uint8_t kImuInt1 = PIN_LSM6DS3TR_C_INT1;
constexpr uint8_t kImuPower = PIN_LSM6DS3TR_C_POWER;
#else
constexpr uint8_t kImuInt1 = 18;
constexpr uint8_t kImuPower = 15;
#endif

} // namespace pin
