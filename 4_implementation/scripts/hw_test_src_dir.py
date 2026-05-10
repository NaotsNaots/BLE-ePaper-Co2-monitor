# PlatformIO 6 では src_dir を [env:] レベルで指定できないため、
# このスクリプトで hw テスト環境の PROJECT_SRC_DIR を上書きする。
Import("env")
import os

HW_TEST_SRC = {
    "test_sgp40_only": "test/hw/sgp40_only",
    "test_epaper_only": "test/hw/epaper_only",
    "test_imu_wake":   "test/hw/imu_wake",
    "test_ble_adv":    "test/hw/ble_adv",
    "test_sleep_cycle":"test/hw/sleep_cycle",
}

env_name = env["PIOENV"]
if env_name in HW_TEST_SRC:
    src_dir = os.path.join(env["PROJECT_DIR"], HW_TEST_SRC[env_name])
    env.Replace(PROJECT_SRC_DIR=src_dir)
