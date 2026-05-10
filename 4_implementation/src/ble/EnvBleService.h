#pragma once

#include <bluefruit.h>
#include "app/types.h"

class EnvBleService {
public:
  void begin(const MeasurementFrame& snap);
  void updateGattSnapshot(const MeasurementFrame& snap);
  void startAdvertising(uint32_t windowMs);
  void stopAdvertising();
  void notifySnapshotIfPossible(const MeasurementFrame& snap);

private:
  BLEService service_;
  BLECharacteristic chrVoc_;
  BLECharacteristic chrEco2_;
  BLECharacteristic chrSeq_;
  BLECharacteristic chrFw_;
  bool svcBegun_ = false;
};
