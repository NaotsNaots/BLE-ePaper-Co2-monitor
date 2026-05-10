#pragma once

#include "app/types.h"

class Eco2Mapper {
public:
  void snapshotFromVoc(int32_t vocIndex, const MeasurementFrame* prev, MeasurementFrame& out);
};
