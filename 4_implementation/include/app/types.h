#pragma once

#include <cstdint>

constexpr uint32_t kWakeIntervalMs = 600000;
constexpr uint32_t kBleAdvertiseWindowMs = 60000;
constexpr uint8_t  kSensorI2cAddressSgp40 = 0x59;
constexpr uint16_t kHistoryCapacity   = 96; // 48h ÷ 30min
constexpr uint8_t  kHistorySampleEvery = 3; // 10min × 3 = 30min に1回保存

enum class WakeReason : uint8_t { Timer10Min, ImuMotion, ColdBoot, Unknown };
enum class VentClass : uint8_t { Ok, Mild, High };
enum class TrendClass : uint8_t { Flat, Up, Down };
enum class BleLinkState : uint8_t { Idle, Advertising, Connected };
enum class Sgp40Error : uint8_t { Ok, I2cNack, SelfTestFail, LowLevel };

struct MeasurementFrame {
  uint32_t sampleIndex;
  uint16_t srawVoc;
  int32_t vocIndex;
  uint16_t eco2Ppm;
  VentClass vent;
  TrendClass trend;
};
