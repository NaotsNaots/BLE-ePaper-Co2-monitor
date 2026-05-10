#include "ImuWakeSource.h"
#include "app/pin_config.h"

bool ImuWakeSource::begin() {
  pinMode(pin::kImuPower, OUTPUT);
  digitalWrite(pin::kImuPower, HIGH);
  Wire1.begin();
  if (!imu_.begin_I2C(LSM6DS_I2CADDR_DEFAULT, &Wire1)) {
    if (!imu_.begin_I2C(0x6B, &Wire1)) {
      return false;
    }
  }
  imu_.setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
  imu_.setAccelDataRate(LSM6DS_RATE_12_5_HZ);
  return true;
}

bool ImuWakeSource::configureWakeOnMotion(uint8_t threshRegs) {
  imu_.configIntOutputs(false, false);
  imu_.enableWakeup(true, 0, threshRegs);
  imu_.configInt1(false, false, false, false, true);
  return true;
}

void ImuWakeSource::clearIrqFlags() {
  // awake() reads WAKE_UP_SRC (0x1B), which is cleared on read.
  (void)imu_.awake();
}
