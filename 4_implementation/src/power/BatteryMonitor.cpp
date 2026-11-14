#include "BatteryMonitor.h"
#include <Arduino.h>

// 電池（単三 1 本、〜1.6V）の＋端子 → R3 10kΩ → C7 → A0。分圧はないので内部基準 3.0V で直接測る。
// 電池が入っていなければ 0 付近になる。
uint16_t BatteryMonitor::sampleMv() {
  analogReference(AR_INTERNAL_3_0);
  analogReadResolution(12);
  uint32_t sum = 0;
  constexpr uint8_t kN = 8;
  for (uint8_t i = 0; i < kN; ++i) {
    sum += analogRead(A0);
  }
  return (uint16_t)((sum / kN) * 3000u / 4095u);
}

bool BatteryMonitor::isLow() { return false; }  // しきい値は未決（要件 F7）。電流と放電曲線の実測後に決める
