// SGP40 のヒーターを切って置いたあと、入れ直してから SRAW が落ち着くまでの時間を測る。
// 本番ファームは 10 分ヒーターを切り、起きた直後に 1 回だけ測るため、その値が妥当かを確かめる。
// ビルド: pio run -e test_sgp40_coldstart
// 出力: "off=<切っていた秒> t=<入れ直してからの秒> sraw=<値>"（1 秒ごと 40 秒）、
//       その前に連続運転での基準値 "ref sraw=<値>"
#include <Arduino.h>
#include <Adafruit_TinyUSB.h>
#include <SensirionI2CSgp40.h>
#include <Wire.h>

namespace {
SensirionI2CSgp40 sgp40;
constexpr uint16_t kRh = 0x8000;
constexpr uint16_t kT = 0x6666;
const uint32_t kOffSeconds[] = {60, 300, 600};

uint16_t measure() {
  uint16_t s = 0;
  if (sgp40.measureRawSignal(kRh, kT, s) != 0) {
    return 0;
  }
  return s;
}
}  // namespace

void setup() {
  Serial.begin(115200);
  Wire.begin();
  Wire.setClock(400000);
  sgp40.begin(Wire);
}

void loop() {
  for (uint32_t off : kOffSeconds) {
    // 基準: 1 秒ごとに 60 秒連続で測り、最後の値を基準とする
    uint16_t ref = 0;
    for (int i = 0; i < 60; ++i) {
      ref = measure();
      delay(1000);
    }
    Serial.printf("ref sraw=%u\n", ref);

    sgp40.turnHeaterOff();
    Serial.printf("heater off %lu s\n", (unsigned long)off);
    delay(off * 1000);

    for (int t = 0; t < 40; ++t) {
      Serial.printf("off=%lu t=%d sraw=%u\n", (unsigned long)off, t, measure());
      delay(1000);
    }
  }
  Serial.println("done");
  while (true) {
    delay(1000);
  }
}
