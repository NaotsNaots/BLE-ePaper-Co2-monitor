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

  // 履歴フレームを順次 Notify する（1 フレーム = 9 バイト packed）
  void notifyHistoryBurst(const MeasurementFrame* frames, uint16_t count);

private:
  BLEService service_;
  BLECharacteristic chrVoc_;
  BLECharacteristic chrEco2_;
  BLECharacteristic chrSeq_;
  BLECharacteristic chrFw_;
  BLECharacteristic chrHistCount_; // 1105: 履歴件数 (uint16 READ)
  BLECharacteristic chrHistData_;  // 1106: 履歴データ (9byte NOTIFY)
  bool svcBegun_ = false;
};
