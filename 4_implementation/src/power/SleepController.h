#pragma once

#include <cstdint>
#include "app/types.h"

class SleepController {
public:
  void configureGpioSenseWakeup(uint32_t pinNumber);
  void armRtcWakeupSeconds(uint32_t sec);
  void enterSystemOffOrDeepSleep();
  WakeReason takeLastWakeReason();

private:
  uint32_t armSeconds_ = 600;
  uint32_t pinNumber_ = 0;
  WakeReason lastWake_ = WakeReason::Unknown;
};
