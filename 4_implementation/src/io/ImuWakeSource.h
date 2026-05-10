#pragma once

#include <Wire.h>
#include <Adafruit_LSM6DS3TRC.h>

class ImuWakeSource {
public:
  bool begin();
  bool configureWakeOnMotion(uint8_t threshRegs);
  void clearIrqFlags();

private:
  Adafruit_LSM6DS3TRC imu_;
};
