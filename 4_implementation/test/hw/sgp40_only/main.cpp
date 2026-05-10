// SGP40（VOC／eCO₂推計用センサ）のみの配線確認用スケッチ。
// ビルド: pio run -e test_sgp40_only
// 期待: シリアルにシリアル番号と SRAW_VOC が定期的に出る（I2C 0x59、D4/D5）。
//
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <SensirionCore.h>
#include <SensirionI2CSgp40.h>
#include <Wire.h>

namespace {
constexpr uint32_t kUsbReadyMs = 800;
SensirionI2CSgp40 sgp40;
constexpr uint16_t kRhTicks = 0x8000;
constexpr uint16_t kTempTicks = 0x6666;
} // namespace

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);  // USB CDC 接続待ち
  (void)kUsbReadyMs;

  Wire.begin();
  Wire.setClock(400000);
  sgp40.begin(Wire);

  uint16_t serialWords[3];
  uint16_t err = sgp40.getSerialNumber(serialWords, 3);
  if (err != 0) {
    char msg[128];
    errorToString(err, msg, sizeof(msg));
    Serial.print("[sgp40] getSerialNumber NG err=");
    Serial.print(err);
    Serial.print(" ");
    Serial.println(msg);
  } else {
    Serial.print("[sgp40] serial 0x");
    for (unsigned i = 0; i < 3; ++i) {
      const uint16_t v = serialWords[i];
      if (v < 4096) Serial.print("0");
      if (v < 256) Serial.print("0");
      if (v < 16) Serial.print("0");
      Serial.print(v, HEX);
    }
    Serial.println();
  }

  Serial.println("[sgp40] loop: measureRawSignal every 2s");
}

void loop() {
  uint16_t sraw = 0;
  uint16_t err = sgp40.measureRawSignal(kRhTicks, kTempTicks, sraw);
  if (err != 0) {
    char msg[128];
    errorToString(err, msg, sizeof(msg));
    Serial.print("[sgp40] measure NG err=");
    Serial.print(err);
    Serial.print(" ");
    Serial.println(msg);
  } else {
    Serial.print("[sgp40] SRAW_VOC=");
    Serial.println(sraw);
  }
  delay(2000);
}
