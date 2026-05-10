// 層1: Native unit test — StateMachine
// 実行: pio test -e native_statemachine
// 検証: interface_spec.md §3.1 E-APP-01/02/03 に対応する状態遷移
//
#include <unity.h>
#include "app/StateMachine.h"

using namespace app_fsm;

static StateMachine sm;

void setUp() { sm.resetToBoot(); }
void tearDown() {}

// ---- BOOT -------------------------------------------------------------------

void test_boot_cold_start_transitions_to_hw_init() {
  TEST_ASSERT_EQUAL_UINT8(BOOT, sm.current());
  sm.onEvent(COLD_START);
  TEST_ASSERT_EQUAL_UINT8(HW_INIT, sm.current());
}

void test_boot_ignores_other_events() {
  sm.onEvent(MEASURE_OK);
  TEST_ASSERT_EQUAL_UINT8(BOOT, sm.current());
  sm.onEvent(BLE_WINDOW_EXPIRED);
  TEST_ASSERT_EQUAL_UINT8(BOOT, sm.current());
}

// ---- RUN_MEASURE ------------------------------------------------------------

void test_measure_ok_transitions_to_run_display() {
  sm.setState(RUN_MEASURE);
  sm.onEvent(MEASURE_OK);
  TEST_ASSERT_EQUAL_UINT8(RUN_DISPLAY, sm.current());
}

void test_measure_err_transitions_to_error_latch() {
  sm.setState(RUN_MEASURE);
  sm.onEvent(MEASURE_ERR);
  TEST_ASSERT_EQUAL_UINT8(ERROR_LATCH, sm.current());
}

void test_measure_ignores_ble_expired() {
  sm.setState(RUN_MEASURE);
  sm.onEvent(BLE_WINDOW_EXPIRED);
  TEST_ASSERT_EQUAL_UINT8(RUN_MEASURE, sm.current());
}

// ---- BLE_WINDOW -------------------------------------------------------------

void test_ble_window_expired_transitions_to_power_down() {
  sm.setState(BLE_WINDOW);
  sm.onEvent(BLE_WINDOW_EXPIRED);  // E-APP-03
  TEST_ASSERT_EQUAL_UINT8(POWER_DOWN, sm.current());
}

void test_ble_window_ignores_measure_ok() {
  sm.setState(BLE_WINDOW);
  sm.onEvent(MEASURE_OK);
  TEST_ASSERT_EQUAL_UINT8(BLE_WINDOW, sm.current());
}

// ---- ERROR_LATCH は変化しない -----------------------------------------------

void test_error_latch_ignores_all() {
  sm.setState(ERROR_LATCH);
  sm.onEvent(COLD_START);
  TEST_ASSERT_EQUAL_UINT8(ERROR_LATCH, sm.current());
  sm.onEvent(MEASURE_OK);
  TEST_ASSERT_EQUAL_UINT8(ERROR_LATCH, sm.current());
  sm.onEvent(BLE_WINDOW_EXPIRED);
  TEST_ASSERT_EQUAL_UINT8(ERROR_LATCH, sm.current());
}

// ---- POWER_DOWN は変化しない ------------------------------------------------

void test_power_down_ignores_all() {
  sm.setState(POWER_DOWN);
  sm.onEvent(WAKE_TIMER_10MIN);
  TEST_ASSERT_EQUAL_UINT8(POWER_DOWN, sm.current());
  sm.onEvent(WAKE_IMU);
  TEST_ASSERT_EQUAL_UINT8(POWER_DOWN, sm.current());
}

// -----------------------------------------------------------------------------

int main() {
  UNITY_BEGIN();

  RUN_TEST(test_boot_cold_start_transitions_to_hw_init);
  RUN_TEST(test_boot_ignores_other_events);

  RUN_TEST(test_measure_ok_transitions_to_run_display);
  RUN_TEST(test_measure_err_transitions_to_error_latch);
  RUN_TEST(test_measure_ignores_ble_expired);

  RUN_TEST(test_ble_window_expired_transitions_to_power_down);
  RUN_TEST(test_ble_window_ignores_measure_ok);

  RUN_TEST(test_error_latch_ignores_all);
  RUN_TEST(test_power_down_ignores_all);

  return UNITY_END();
}
