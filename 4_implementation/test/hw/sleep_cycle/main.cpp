// 層3: スリープ・ウェークサイクル通しテスト
// ビルド: pio run -e test_sleep_cycle -t upload && pio device monitor -e test_sleep_cycle
// 検証: interface_spec.md §3.2/3.3/3.7 — E-TMR-01、E-IMU-01、OP-SLP-01/02、E-SLP-01
//
// 目的:
//   本番は 600秒周期でテストが困難なため、RTC を 5秒に短縮して複数サイクルを観察する。
//   WakeReason が Serial に出ることで復帰原因が正しく識別できることを確認する。
//
// 期待する Serial 出力（例）:
//   [wake] reason=ColdBoot
//   [meas] vocIndex=120 eco2=880ppm
//   [epd]  refresh done
//   [sleep] entering WFE deep sleep (5s)
//   [wake] reason=Timer10Min         ← 5秒後
//   ...
//   [wake] reason=ImuMotion          ← 振動させた場合
//
// 合否判定:
//   - [wake] reason= が Timer10Min / ImuMotion のいずれかで出ること
//   - [meas] が毎サイクル出ること
//   - [epd] refresh done が毎サイクル出ること
//
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <SPI.h>
#include <Wire.h>
#include <SensirionCore.h>
#include <SensirionI2CSgp40.h>
#include <VOCGasIndexAlgorithm.h>
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>
#include <Adafruit_LSM6DS3TRC.h>

#include "app/pin_config.h"
#include "app/types.h"
#include "display/epd_driver.h"
#include "power/SleepController.h"
#include "io/ImuWakeSource.h"

// スリープ時間（本番=600s → テスト=5s）
static constexpr uint32_t kTestSleepSec = 5;

namespace {

SensirionI2CSgp40 sgp40;
VOCGasIndexAlgorithm vocAlgo(kTestSleepSec);  // テスト間隔に合わせる
GxEPD2_BW<EpdDriver, kEpdHeight> display(
    EpdDriver(pin::kEpdCs, pin::kEpdDc, pin::kEpdRst, pin::kEpdBusy));
SleepController sleepCtrl;
ImuWakeSource imu;
WakeReason wakeReason = WakeReason::ColdBoot;
uint32_t sampleIndex = 0;

const char* wakeReasonStr(WakeReason r) {
  switch (r) {
  case WakeReason::ColdBoot: return "ColdBoot";
  case WakeReason::Timer10Min: return "Timer10Min";
  case WakeReason::ImuMotion: return "ImuMotion";
  default: return "Unknown";
  }
}

void doMeasure(MeasurementFrame& frame) {
  uint16_t sraw = 0;
  constexpr uint16_t kRhTicks = 0x8000;
  constexpr uint16_t kTempTicks = 0x6666;
  sgp40.measureRawSignal(kRhTicks, kTempTicks, sraw);
  frame.srawVoc = sraw;
  frame.vocIndex = vocAlgo.process(sraw);
  const int32_t c = (frame.vocIndex < 0) ? 0 : (frame.vocIndex > 500 ? 500 : frame.vocIndex);
  frame.eco2Ppm = (uint16_t)(400u + (uint32_t)c * 4u > 2000u
                                 ? 2000u
                                 : 400u + (uint32_t)c * 4u);
  frame.sampleIndex = ++sampleIndex;
}

void doDisplay(const MeasurementFrame& frame) {
  char line[48];
  display.setRotation(3);
  display.setFont(&FreeMonoBold9pt7b);
  display.setTextColor(GxEPD_BLACK);
  display.setFullWindow();
  display.firstPage();
  do {
    display.fillScreen(GxEPD_WHITE);
    display.setCursor(8, 32);
    snprintf(line, sizeof(line), "VOC %d", (int)frame.vocIndex);
    display.print(line);
    display.setCursor(8, 56);
    snprintf(line, sizeof(line), "eCO2~ %u", (unsigned)frame.eco2Ppm);
    display.print(line);
    display.setCursor(8, 80);
    snprintf(line, sizeof(line), "#%lu", (unsigned long)frame.sampleIndex);
    display.print(line);
  } while (display.nextPage());
}

} // namespace

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  // SGP40
  Wire.begin();
  Wire.setClock(400000);
  sgp40.begin(Wire);

  // IMU
  if (!imu.begin()) {
    Serial.println("[NG] IMU begin failed");
  } else {
    imu.configureWakeOnMotion(4);
    Serial.println("[OK] IMU configured");
  }

  // e-Paper
  SPI.begin();
  display.init(0, true);
  Serial.println("[OK] e-Paper initialized");
}

void loop() {
  Serial.print("[wake] reason=");
  Serial.println(wakeReasonStr(wakeReason));

  // 測定
  MeasurementFrame frame{};
  doMeasure(frame);
  Serial.print("[meas] vocIndex=");
  Serial.print(frame.vocIndex);
  Serial.print(" eco2=");
  Serial.print(frame.eco2Ppm);
  Serial.println("ppm");

  // 表示
  doDisplay(frame);
  Serial.println("[epd]  refresh done");

  // スリープ前処理
  imu.clearIrqFlags();
  display.hibernate();
  Wire.end();
  Wire1.end();
  SPI.end();

  sleepCtrl.configureGpioSenseWakeup(pin::kImuInt1);
  sleepCtrl.armRtcWakeupSeconds(kTestSleepSec);

  Serial.print("[sleep] entering WFE deep sleep (");
  Serial.print(kTestSleepSec);
  Serial.println("s)");
  Serial.flush();

  sleepCtrl.enterSystemOffOrDeepSleep();

  // ウェーク後の再初期化
  Wire.begin();
  Wire.setClock(400000);
  Wire1.begin();
  SPI.begin();
  display.init(0, false);

  wakeReason = sleepCtrl.takeLastWakeReason();
}
