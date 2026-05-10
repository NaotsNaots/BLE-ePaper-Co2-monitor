#include "Eco2Mapper.h"

// interface_spec §3.5: eCO₂ は参考推計（真 CO₂ 非測定）。係数は製品側チューニング用。
namespace {

int32_t clampVoc(int32_t v) {
  if (v < 0) {
    return 0;
  }
  if (v > 500) {
    return 500;
  }
  return v;
}

// 単調だが線形とは限らない簡易マップ（LE uint16 ppm 相当で BLE/表示と整合）
uint16_t vocToEco2Ppm(int32_t voc) {
  const int32_t c = clampVoc(voc);
  uint32_t ppm = 400u + (uint32_t)c * 4u;
  if (ppm > 2000u) {
    ppm = 2000u;
  }
  return (uint16_t)ppm;
}

VentClass ventFromVoc(int32_t voc) {
  const int32_t c = clampVoc(voc);
  if (c >= 220) {
    return VentClass::High;
  }
  if (c >= 120) {
    return VentClass::Mild;
  }
  return VentClass::Ok;
}

TrendClass trendFromDelta(int32_t prevVoc, int32_t curVoc) {
  const int32_t d = curVoc - prevVoc;
  if (d > 15) {
    return TrendClass::Up;
  }
  if (d < -15) {
    return TrendClass::Down;
  }
  return TrendClass::Flat;
}

} // namespace

void Eco2Mapper::snapshotFromVoc(int32_t vocIndex, const MeasurementFrame* prev, MeasurementFrame& out) {
  out.vocIndex = vocIndex;
  out.eco2Ppm = vocToEco2Ppm(vocIndex);
  out.vent = ventFromVoc(vocIndex);
  if (prev != nullptr) {
    out.trend = trendFromDelta(prev->vocIndex, vocIndex);
  } else {
    out.trend = TrendClass::Flat;
  }
}
