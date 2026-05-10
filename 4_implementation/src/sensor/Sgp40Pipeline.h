#pragma once

#include <Wire.h>
#include <SensirionI2CSgp40.h>
#include <VOCGasIndexAlgorithm.h>
#include "app/types.h"

class Sgp40Pipeline {
public:
  bool begin(TwoWire& w);
  Sgp40Error measureVocIndex(uint16_t& srawOut, int32_t& vocOut);
  void heaterOffForSleep();
  bool runOptionalSelfTest();

private:
  SensirionI2CSgp40 sgp_;
  VOCGasIndexAlgorithm vocAlg_{600.0f};
};
