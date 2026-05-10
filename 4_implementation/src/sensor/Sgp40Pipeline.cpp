#include "Sgp40Pipeline.h"

namespace {
constexpr uint16_t kRhTicksDefault = 0x8000;
constexpr uint16_t kTempTicksDefault = 0x6666;
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
  uint16_t err = sgp_.measureRawSignal(kRhTicksDefault, kTempTicksDefault, srawOut);
  if (err != 0) {
    return Sgp40Error::I2cNack;
  }
  vocOut = vocAlg_.process((int32_t)srawOut);
  return Sgp40Error::Ok;
}

void Sgp40Pipeline::heaterOffForSleep() {
  (void)sgp_.turnHeaterOff();
}
