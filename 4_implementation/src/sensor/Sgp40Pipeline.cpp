#include "Sgp40Pipeline.h"

namespace {
constexpr uint16_t kRhTicksDefault = 0x8000;
constexpr uint16_t kTempTicksDefault = 0x6666;
constexpr uint8_t kPreheatSeconds = 30;  // 温める回数（1 秒ごと）。長いほど正確、電池は減る
constexpr uint8_t kAverageLast = 5;      // 最後の何回を平均するか（ばらつき低減）
} // namespace

bool Sgp40Pipeline::begin(TwoWire& w) {
  sgp_.begin(w);
  return true;
}

bool Sgp40Pipeline::runOptionalSelfTest() {
  uint16_t st = 0;
  uint16_t err = sgp_.executeSelfTest(st);
  return err == 0 && st == 0xD400;
}

Sgp40Error Sgp40Pipeline::measureVocIndex(uint16_t& srawOut, int32_t& vocOut) {
  // ヒーターを切っていた後の 1 回目は必ず SRAW=0 が返る（実測）。VOC アルゴリズムは 0 を捨てるため、
  // 1 回だけ測ると指数が初期値 100 から動かない。さらに値が落ち着くまで 20〜30 秒かかるので、
  // 1 秒ごとに kPreheatSeconds 回測ってヒーターを温め、最後の kAverageLast 回の平均を使う（2026-11-14 実機確認。class_diagram.md §6.1 / SCOPE.md）。
  // 10 秒予熱では起床ごとの値が ±200 ほどぶれ、学習初期の指数が 1〜296 と振れて誤報が出た（BRINGUP.md）。
  uint32_t sum = 0;
  uint8_t n = 0;
  uint16_t sraw = 0;
  for (uint8_t i = 0; i < kPreheatSeconds; ++i) {
    if (i > 0) {
      delay(1000);
    }
    uint16_t err = sgp_.measureRawSignal(kRhTicksDefault, kTempTicksDefault, sraw);
    if (err != 0) {
      return Sgp40Error::I2cNack;
    }
    if (i >= kPreheatSeconds - kAverageLast && sraw != 0) {
      sum += sraw;
      ++n;
    }
  }
  if (n == 0) {
    return Sgp40Error::LowLevel;
  }
  sraw = (uint16_t)((sum + n / 2) / n);
  srawOut = sraw;
  vocOut = vocAlg_.process((int32_t)srawOut);
  return Sgp40Error::Ok;
}

void Sgp40Pipeline::heaterOffForSleep() {
  (void)sgp_.turnHeaterOff();
}
