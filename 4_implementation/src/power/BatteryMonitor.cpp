#include "BatteryMonitor.h"
#include <Arduino.h>

uint16_t BatteryMonitor::sampleMv() { return 3300; }

bool BatteryMonitor::isLow() { return false; }
