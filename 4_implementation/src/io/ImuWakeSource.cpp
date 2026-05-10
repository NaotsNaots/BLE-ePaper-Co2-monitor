#include "ImuWakeSource.h"
#include "app/pin_config.h"

static void recoverWire1Bus() {
  // nRF52840 TWIM はパワーオン直後に SDA がスタックしていると
  // endTransmission() で無限ハングする。SCL 9 クロックで解放する。
  constexpr uint8_t kScl = 16;  // D16 = P0.27
  constexpr uint8_t kSda = 17;  // D17 = P0.07
  pinMode(kScl, OUTPUT); pinMode(kSda, OUTPUT);
  digitalWrite(kScl, HIGH); digitalWrite(kSda, HIGH); delay(10);
  for (int i = 0; i < 9; i++) {
    digitalWrite(kScl, LOW);  delayMicroseconds(5);
    digitalWrite(kScl, HIGH); delayMicroseconds(5);
  }
  digitalWrite(kSda, LOW); delayMicroseconds(5);
  digitalWrite(kScl, HIGH); delayMicroseconds(5);
  digitalWrite(kSda, HIGH); delay(10);
  pinMode(kScl, INPUT); pinMode(kSda, INPUT);
}

bool ImuWakeSource::begin() {
  pinMode(pin::kImuPower, OUTPUT);
  digitalWrite(pin::kImuPower, HIGH);
  delay(200);  // IMU パワーオン安定待ち
  recoverWire1Bus();
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
