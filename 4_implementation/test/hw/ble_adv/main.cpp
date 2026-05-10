// 層2: BLE 広告・GATT Read テスト
// ビルド: pio run -e test_ble_adv -t upload && pio device monitor -e test_ble_adv
// 検証: interface_spec.md §3.8 OP-BLE-01〜03、D-BLE-01
//
// 合否判定:
//   1. Serial に "[OK] BLE started" が出ること
//   2. スマホの nRF Connect で UUID "11111101-...-0001" のサービスを発見できること
//   3. 各特性を Read して期待値（下記）が返ること
//      - VOC:  0x0078 (120 LE)
//      - eCO2: 0x0380 (896 LE) — vocToEco2(120) = 400+120*4 = 880
//      - SEQ:  0x00000001
//      - FW:   "0.1.0" (ASCII)
//   4. CCCD を Notify 有効化すると 3秒ごとに Notify が届くこと
//
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <bluefruit.h>
#include "app/types.h"
#include "ble/EnvBleService.h"

namespace {

EnvBleService bleService;

MeasurementFrame makeTestFrame() {
  MeasurementFrame f{};
  f.sampleIndex = 1;
  f.vocIndex = 120;
  f.eco2Ppm = 880;  // vocToEco2(120)
  f.vent = VentClass::Mild;
  f.trend = TrendClass::Flat;
  return f;
}

void connectCallback(uint16_t /*handle*/) {
  Serial.println("[D-BLE-01] Central connected");
}

void disconnectCallback(uint16_t /*handle*/, uint8_t reason) {
  Serial.print("[D-BLE-01] Disconnected, reason=0x");
  Serial.println(reason, HEX);
}

} // namespace

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Bluefruit.begin();
  Bluefruit.setName("EnvMon-Test");
  Bluefruit.Periph.setConnectCallback(connectCallback);
  Bluefruit.Periph.setDisconnectCallback(disconnectCallback);
  Serial.println("[OK] Bluefruit.begin");

  MeasurementFrame frame = makeTestFrame();
  bleService.begin(frame);
  Serial.println("[OK] EnvBleService.begin");

  bleService.startAdvertising(0);
  Serial.println("[OK] BLE advertising started");
  Serial.println("--- Connect with nRF Connect and Read each characteristic ---");
  Serial.print("    Service UUID: 11111101-2222-4333-8444-A55555550001");
  Serial.println();
}

void loop() {
  static uint32_t lastMs = 0;
  const uint32_t now = millis();
  if (now - lastMs >= 3000) {
    lastMs = now;
    MeasurementFrame frame = makeTestFrame();
    frame.sampleIndex++;
    bleService.notifySnapshotIfPossible(frame);
    Serial.print("[notify] seq=");
    Serial.println(frame.sampleIndex);
  }
}
