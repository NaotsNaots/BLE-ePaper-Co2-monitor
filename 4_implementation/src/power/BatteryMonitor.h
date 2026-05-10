#pragma once

#include <cstdint>

class BatteryMonitor {
public:
  uint16_t sampleMv();
  bool isLow();
};
