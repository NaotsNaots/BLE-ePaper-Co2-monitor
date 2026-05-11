#pragma once

#include "app/types.h"

class HistoryBuffer {
public:
  void push(const MeasurementFrame& f);
  uint16_t count() const { return count_; }
  const MeasurementFrame& at(uint16_t i) const; // 0 = oldest

private:
  MeasurementFrame buf_[kHistoryCapacity];
  uint16_t head_  = 0;
  uint16_t count_ = 0;
};
