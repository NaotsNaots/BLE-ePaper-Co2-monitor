#pragma once

#include "app/StateMachine.h"
#include "app/types.h"
#include "sensor/Sgp40Pipeline.h"
#include "sensor/Eco2Mapper.h"
#include "display/EpdPresenter.h"
#include "io/ImuWakeSource.h"
#include "power/SleepController.h"
#include "power/BatteryMonitor.h"
#include "ble/EnvBleService.h"

class ApplicationController {
public:
  void setup();
  void loop();

private:
  bool runHwInit();
  void runMeasurePipeline();
  void runBleWindowIfNeeded();
  void armSleepAndYield();

  StateMachine sm_;
  Sgp40Pipeline sgp_;
  Eco2Mapper eco2_;
  EpdPresenter epd_;
  ImuWakeSource imu_;
  SleepController sleep_;
  BatteryMonitor battery_;
  EnvBleService ble_;

  bool hwReady_ = false;
  bool firstCycle_ = true;
  bool bleSessionActive_ = false;
  uint32_t bleWindowStartMs_ = 0;
  MeasurementFrame lastFrame_{};
  uint32_t sampleSeq_ = 0;
};
