#include "app/StateMachine.h"

void StateMachine::onEvent(app_fsm::Event ev) {
  using namespace app_fsm;
  switch (state_) {
  case BOOT:
    if (ev == COLD_START) {
      state_ = HW_INIT;
    }
    break;
  case HW_INIT:
    break;
  case RUN_MEASURE:
    if (ev == MEASURE_OK) {
      state_ = RUN_DISPLAY;
    } else if (ev == MEASURE_ERR) {
      state_ = ERROR_LATCH;
    }
    break;
  case RUN_DISPLAY:
    break;
  case BLE_WINDOW:
    if (ev == BLE_WINDOW_EXPIRED) {
      state_ = POWER_DOWN;
    }
    break;
  case POWER_DOWN:
  case LOW_BATTERY_HALT:
  case ERROR_LATCH:
    break;
  }
}
