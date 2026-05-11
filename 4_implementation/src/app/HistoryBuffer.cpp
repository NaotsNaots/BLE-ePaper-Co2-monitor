#include "app/HistoryBuffer.h"

void HistoryBuffer::push(const MeasurementFrame& f) {
  buf_[head_] = f;
  head_ = (head_ + 1) % kHistoryCapacity;
  if (count_ < kHistoryCapacity) ++count_;
}

const MeasurementFrame& HistoryBuffer::at(uint16_t i) const {
  uint16_t idx = (count_ < kHistoryCapacity)
    ? i
    : static_cast<uint16_t>((head_ + i) % kHistoryCapacity);
  return buf_[idx];
}
