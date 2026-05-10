#pragma once

#include <cstdint>

// State / event IDs match 3_detailed_design/state_machine.md (names abbreviated).

namespace app_fsm {

enum State : uint8_t {
  BOOT,
  HW_INIT,
  RUN_MEASURE,
  RUN_DISPLAY,
  BLE_WINDOW,
  POWER_DOWN,
  LOW_BATTERY_HALT,
  ERROR_LATCH,
};

enum Event : uint8_t {
  COLD_START,
  WAKE_TIMER_10MIN,
  WAKE_IMU,
  MEASURE_OK,
  MEASURE_ERR,
  DISPLAY_DONE,
  BLE_WINDOW_EXPIRED,
  LOW_BATTERY,
  FORCE_BLE,
};

} // namespace app_fsm

class StateMachine {
public:
  app_fsm::State current() const { return state_; }
  void resetToBoot() { state_ = app_fsm::BOOT; }
  void onEvent(app_fsm::Event ev);
  void setState(app_fsm::State s) { state_ = s; }

private:
  app_fsm::State state_ = app_fsm::BOOT;
};
