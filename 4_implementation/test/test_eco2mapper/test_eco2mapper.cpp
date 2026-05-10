// 層1: Native unit test — Eco2Mapper
// 実行: pio test -e native_eco2mapper
// 検証: interface_spec.md §3.5 OP-ECO2-01（スナップショット生成）
//
#include <unity.h>
#include "sensor/Eco2Mapper.h"

static Eco2Mapper mapper;

void setUp() {}
void tearDown() {}

// ---- eCO₂ 変換 ----------------------------------------------------------------

void test_voc0_gives_baseline_eco2() {
  MeasurementFrame out{};
  mapper.snapshotFromVoc(0, nullptr, out);
  TEST_ASSERT_EQUAL_UINT16(400, out.eco2Ppm);
}

void test_voc100_eco2() {
  MeasurementFrame out{};
  mapper.snapshotFromVoc(100, nullptr, out);
  TEST_ASSERT_EQUAL_UINT16(800, out.eco2Ppm);  // 400 + 100*4
}

void test_voc400_eco2() {
  MeasurementFrame out{};
  mapper.snapshotFromVoc(400, nullptr, out);
  TEST_ASSERT_EQUAL_UINT16(2000, out.eco2Ppm);  // 400 + 400*4 = 2000
}

void test_voc500_clamps_eco2() {
  MeasurementFrame out{};
  mapper.snapshotFromVoc(500, nullptr, out);
  TEST_ASSERT_EQUAL_UINT16(2000, out.eco2Ppm);  // 上限クランプ
}

void test_negative_voc_clamps_to_baseline() {
  MeasurementFrame out{};
  mapper.snapshotFromVoc(-10, nullptr, out);
  TEST_ASSERT_EQUAL_UINT16(400, out.eco2Ppm);
}

// ---- VentClass ---------------------------------------------------------------

void test_vent_ok_below_120() {
  MeasurementFrame out{};
  mapper.snapshotFromVoc(119, nullptr, out);
  TEST_ASSERT_EQUAL_UINT8((uint8_t)VentClass::Ok, (uint8_t)out.vent);
}

void test_vent_mild_at_120() {
  MeasurementFrame out{};
  mapper.snapshotFromVoc(120, nullptr, out);
  TEST_ASSERT_EQUAL_UINT8((uint8_t)VentClass::Mild, (uint8_t)out.vent);
}

void test_vent_mild_at_219() {
  MeasurementFrame out{};
  mapper.snapshotFromVoc(219, nullptr, out);
  TEST_ASSERT_EQUAL_UINT8((uint8_t)VentClass::Mild, (uint8_t)out.vent);
}

void test_vent_high_at_220() {
  MeasurementFrame out{};
  mapper.snapshotFromVoc(220, nullptr, out);
  TEST_ASSERT_EQUAL_UINT8((uint8_t)VentClass::High, (uint8_t)out.vent);
}

// ---- TrendClass --------------------------------------------------------------

void test_trend_up_delta_over_15() {
  MeasurementFrame prev{};
  prev.vocIndex = 100;
  MeasurementFrame out{};
  mapper.snapshotFromVoc(116, &prev, out);  // delta=16
  TEST_ASSERT_EQUAL_UINT8((uint8_t)TrendClass::Up, (uint8_t)out.trend);
}

void test_trend_flat_delta_15() {
  MeasurementFrame prev{};
  prev.vocIndex = 100;
  MeasurementFrame out{};
  mapper.snapshotFromVoc(115, &prev, out);  // delta=15 → Flat（境界は含まない）
  TEST_ASSERT_EQUAL_UINT8((uint8_t)TrendClass::Flat, (uint8_t)out.trend);
}

void test_trend_down_delta_under_minus15() {
  MeasurementFrame prev{};
  prev.vocIndex = 200;
  MeasurementFrame out{};
  mapper.snapshotFromVoc(183, &prev, out);  // delta=-17
  TEST_ASSERT_EQUAL_UINT8((uint8_t)TrendClass::Down, (uint8_t)out.trend);
}

void test_trend_flat_delta_minus15() {
  MeasurementFrame prev{};
  prev.vocIndex = 200;
  MeasurementFrame out{};
  mapper.snapshotFromVoc(185, &prev, out);  // delta=-15 → Flat
  TEST_ASSERT_EQUAL_UINT8((uint8_t)TrendClass::Flat, (uint8_t)out.trend);
}

void test_no_prev_is_flat() {
  MeasurementFrame out{};
  mapper.snapshotFromVoc(300, nullptr, out);
  TEST_ASSERT_EQUAL_UINT8((uint8_t)TrendClass::Flat, (uint8_t)out.trend);
}

// ---- vocIndex パススルー -----------------------------------------------------

void test_vocindex_stored_in_out() {
  MeasurementFrame out{};
  mapper.snapshotFromVoc(123, nullptr, out);
  TEST_ASSERT_EQUAL_INT32(123, out.vocIndex);
}

// -----------------------------------------------------------------------------

int main() {
  UNITY_BEGIN();

  RUN_TEST(test_voc0_gives_baseline_eco2);
  RUN_TEST(test_voc100_eco2);
  RUN_TEST(test_voc400_eco2);
  RUN_TEST(test_voc500_clamps_eco2);
  RUN_TEST(test_negative_voc_clamps_to_baseline);

  RUN_TEST(test_vent_ok_below_120);
  RUN_TEST(test_vent_mild_at_120);
  RUN_TEST(test_vent_mild_at_219);
  RUN_TEST(test_vent_high_at_220);

  RUN_TEST(test_trend_up_delta_over_15);
  RUN_TEST(test_trend_flat_delta_15);
  RUN_TEST(test_trend_down_delta_under_minus15);
  RUN_TEST(test_trend_flat_delta_minus15);
  RUN_TEST(test_no_prev_is_flat);

  RUN_TEST(test_vocindex_stored_in_out);

  return UNITY_END();
}
