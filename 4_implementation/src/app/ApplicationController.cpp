#include "app/ApplicationController.h"
#include "app/pin_config.h"
#include <Adafruit_TinyUSB.h>
#include <Wire.h>
#include <SPI.h>
#include <bluefruit.h>

namespace {

constexpr uint32_t kSerialBaud = 115200;
constexpr uint32_t kUsbDelayMs = 800;
constexpr uint8_t kImuThresh = 4;
constexpr uint8_t kMeasureRetries = 3;
constexpr uint32_t kWakeSeconds = kWakeIntervalMs / 1000u;

} // namespace

bool ApplicationController::runHwInit() {
  sm_.resetToBoot();
  sm_.onEvent(app_fsm::COLD_START);

  Wire.begin();
  Wire.setClock(400000);
  if (!sgp_.begin(Wire)) {
    return false;
  }

#if defined(SGP40_RUN_SELFTEST)
  if (!sgp_.runOptionalSelfTest()) {
    return false;
  }
#endif

  if (!epd_.begin()) {
    return false;
  }

  if (!imu_.begin()) {
    return false;
  }
  if (!imu_.configureWakeOnMotion(kImuThresh)) {
    return false;
  }

  Bluefruit.begin();
  Bluefruit.setName("EnvEpd");
  Bluefruit.Periph.setConnectCallback(
      [](uint16_t) { Serial.println("[ble] connected"); });
  Bluefruit.Periph.setDisconnectCallback(
      [](uint16_t, uint8_t) { Serial.println("[ble] disconnected"); });

  MeasurementFrame stub{};
  ble_.begin(stub);

  sm_.setState(app_fsm::HW_INIT);
  return true;
}

void ApplicationController::runMeasurePipeline() {
  sm_.setState(app_fsm::RUN_MEASURE);
  uint16_t sraw = 0;
  int32_t voc = 0;
  Sgp40Error err = Sgp40Error::LowLevel;
  for (uint8_t a = 0; a < kMeasureRetries; ++a) {
    err = sgp_.measureVocIndex(sraw, voc);
    if (err == Sgp40Error::Ok) {
      break;
    }
    delay(50);
  }
  if (err != Sgp40Error::Ok) {
    Serial.println("[meas] SGP40 error");
    sm_.onEvent(app_fsm::MEASURE_ERR);
    sm_.setState(app_fsm::ERROR_LATCH);
    epd_.showError("Sensor fault", "Check SGP40 I2C");
    sgp_.heaterOffForSleep();
    epd_.hibernatePanel();
    sleep_.configureGpioSenseWakeup(pin::kImuInt1);
    sleep_.armRtcWakeupSeconds(kWakeSeconds);
    sleep_.enterSystemOffOrDeepSleep();
    return;
  }

  MeasurementFrame frame{};
  frame.sampleIndex = ++sampleSeq_;
  frame.srawVoc = sraw;
  eco2_.snapshotFromVoc(voc, (sampleSeq_ > 1) ? &lastFrame_ : nullptr, frame);
  lastFrame_ = frame;

  // 3 サイクル（約30分）に1回履歴バッファへ保存
  if (++histCycleCount_ >= kHistorySampleEvery) {
    hist_.push(frame);
    histCycleCount_ = 0;
  }

  sm_.onEvent(app_fsm::MEASURE_OK);
}

void ApplicationController::armSleepAndYield() {
  imu_.clearIrqFlags();
  sgp_.heaterOffForSleep();
  epd_.hibernatePanel();
  // I²C/SPI を停止して入眠中の消費電流を削減。
  // IMU は configureWakeOnMotion() 後は自律動作するため停止可。
  Wire.end();
  Wire1.end();
  SPI.end();
  sleep_.configureGpioSenseWakeup(pin::kImuInt1);
  sleep_.armRtcWakeupSeconds(kWakeSeconds);
  sleep_.enterSystemOffOrDeepSleep();
  // 復帰後は Wire/SPI を再初期化してから次サイクルへ進む。
  Wire.begin();
  Wire.setClock(400000);
  Wire1.begin();
  SPI.begin();
}

void ApplicationController::runBleWindowIfNeeded() {
  if (!bleSessionActive_) {
    return;
  }

  // 接続後 CCCD 購読済みになってから1回だけ履歴バースト送信
  // notifyHistoryBurst() が false（未購読）の間はフラグを立てず毎ループ再試行
  if (!historyNotifiedThisWindow_ && Bluefruit.connected()) {
    ble_.updateGattSnapshot(lastFrame_);
    ble_.notifySnapshotIfPossible(lastFrame_);
    bool sent = ble_.notifyHistoryBurst(
      hist_.count() > 0 ? &hist_.at(0) : nullptr, hist_.count());
    if (sent) {
      historyNotifiedThisWindow_ = true;
    }
  }

  uint32_t now = millis();
  if (now - bleWindowStartMs_ >= kBleAdvertiseWindowMs) {
    ble_.stopAdvertising();
    bleSessionActive_ = false;
    historyNotifiedThisWindow_ = false;
    sm_.onEvent(app_fsm::BLE_WINDOW_EXPIRED);
    sm_.setState(app_fsm::POWER_DOWN);
    armSleepAndYield();
  }
}

void ApplicationController::setup() {
  Serial.begin(kSerialBaud);
  delay(kUsbDelayMs);

  Serial.println("[boot] ApplicationController::setup");

  hwReady_ = runHwInit();
  if (!hwReady_) {
    Serial.println("[boot] HW_INIT failed");
    epd_.begin();
    epd_.showError("Init failed", "HW_INIT");
    while (1) {
      delay(1000);
    }
  }
}

void ApplicationController::loop() {
  if (!hwReady_) {
    return;
  }

  if (bleSessionActive_) {
    runBleWindowIfNeeded();
    return;
  }

  WakeReason wr;
  if (firstCycle_) {
    wr = WakeReason::ColdBoot;
    firstCycle_ = false;
  } else {
    wr = sleep_.takeLastWakeReason();
    if (wr == WakeReason::Unknown) {
      wr = WakeReason::Timer10Min;
    }
  }

  if (battery_.isLow()) {
    Serial.println("[bat] low (stub)");
  }

  runMeasurePipeline();
  if (sm_.current() == app_fsm::ERROR_LATCH) {
    return;
  }

  sm_.setState(app_fsm::RUN_DISPLAY);
  epd_.show(lastFrame_);

  const bool doBle = (wr == WakeReason::ImuMotion);
  if (doBle) {
    sm_.setState(app_fsm::BLE_WINDOW);
    ble_.updateGattSnapshot(lastFrame_);
    ble_.startAdvertising(kBleAdvertiseWindowMs);
    bleSessionActive_ = true;
    bleWindowStartMs_ = millis();
    historyNotifiedThisWindow_ = false;
    return;
  }

  sm_.setState(app_fsm::POWER_DOWN);
  armSleepAndYield();
}
