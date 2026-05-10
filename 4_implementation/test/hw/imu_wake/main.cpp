// 層2: IMU ウェーク構成テスト
// ビルド: pio run -e test_imu_wake -t upload && pio device monitor -e test_imu_wake
// 検証: interface_spec.md §3.3 OP-IMU-01（検知構成の適用）、E-IMU-01（運動ウェーク検知）
//
// 合否判定:
//   [OK] が全行に出れば Pass
//   ボードを振ると "WAKE_UP detected" が連続して出ること
//
// ハマりポイント:
//   Wire1.begin() 前に I2C バス回復（SCL 9 クロック）が必要。
//   nRF52840 TWIM はパワーオン直後に SDA がスタックしていると
//   最初の endTransmission() で無限ハングする。
//
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <Adafruit_LSM6DS3TRC.h>
#include <Wire.h>
#include "app/pin_config.h"

namespace {
constexpr uint8_t kWakeThresh = 4;
constexpr uint8_t kWakeDuration = 0;
constexpr uint8_t kWire1Scl = 16;  // D16 = P0.27
constexpr uint8_t kWire1Sda = 17;  // D17 = P0.07
Adafruit_LSM6DS3TRC* imu = nullptr;
} // namespace

static void recoverI2cBus() {
  pinMode(kWire1Scl, OUTPUT); pinMode(kWire1Sda, OUTPUT);
  digitalWrite(kWire1Scl, HIGH); digitalWrite(kWire1Sda, HIGH); delay(10);
  for (int i = 0; i < 9; i++) {
    digitalWrite(kWire1Scl, LOW);  delayMicroseconds(5);
    digitalWrite(kWire1Scl, HIGH); delayMicroseconds(5);
  }
  digitalWrite(kWire1Sda, LOW); delayMicroseconds(5);
  digitalWrite(kWire1Scl, HIGH); delayMicroseconds(5);
  digitalWrite(kWire1Sda, HIGH); delay(10);
  pinMode(kWire1Scl, INPUT); pinMode(kWire1Sda, INPUT);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  pinMode(pin::kImuPower, OUTPUT);
  digitalWrite(pin::kImuPower, HIGH);
  delay(200);  // IMU パワーオン安定待ち

  recoverI2cBus();
  Wire1.begin();

  imu = new Adafruit_LSM6DS3TRC();

  if (!imu->begin_I2C(LSM6DS_I2CADDR_DEFAULT, &Wire1) &&
      !imu->begin_I2C(0x6B, &Wire1)) {
    Serial.println("[NG] IMU begin_I2C failed"); Serial.flush();
    return;
  }
  Serial.println("[OK] IMU begin_I2C");

  imu->setAccelRange(LSM6DS_ACCEL_RANGE_2_G);
  imu->setAccelDataRate(LSM6DS_RATE_12_5_HZ);
  Serial.println("[OK] IMU accel config");

  imu->configIntOutputs(false, false);
  imu->enableWakeup(true, kWakeDuration, kWakeThresh);
  imu->configInt1(false, false, false, false, true);
  Serial.println("[OK] IMU wake-on-motion configured");

  pinMode(pin::kImuInt1, INPUT);
  Serial.print("[OK] kImuInt1 pin=");
  Serial.print(pin::kImuInt1);
  Serial.print(" level=");
  Serial.println(digitalRead(pin::kImuInt1));

  Serial.println("--- Shake the board to trigger WAKE_UP ---");
}

void loop() {
  if (!imu) {
    delay(1000);
    return;
  }
  if (imu->awake()) {
    Serial.println("[E-IMU-01] WAKE_UP detected");
  }
  delay(100);
}
