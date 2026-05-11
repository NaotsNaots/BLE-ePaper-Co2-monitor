# PlatformIO 6 では src_dir を [env:] レベルで指定できないため、
# このスクリプトで hw テスト環境の PROJECT_SRC_DIR を上書きする。
# lib_extra_dirs の LDF は "ble/EnvBleService.h" のようなパス付きインクルードを
# ライブラリとして認識できないため、必要な src/ サブディレクトリを明示的にビルドへ追加する。
Import("env")
import os
import glob

HW_TEST_SRC = {
    "test_sgp40_only": "test/hw/sgp40_only",
    "test_epaper_only": "test/hw/epaper_only",
    "test_imu_wake":   "test/hw/imu_wake",
    "test_ble_adv":    "test/hw/ble_adv",
    "test_sleep_cycle":"test/hw/sleep_cycle",
}

# src/ の追加コンパイル対象（ファイル単位で必要なものだけ指定）
HW_TEST_EXTRA_SRC = {
    "test_ble_adv":    ["src/ble/EnvBleService.cpp"],
    "test_sleep_cycle":["src/power/SleepController.cpp", "src/io/ImuWakeSource.cpp"],
}

env_name = env["PIOENV"]
if env_name in HW_TEST_SRC:
    src_dir = os.path.join(env["PROJECT_DIR"], HW_TEST_SRC[env_name])
    env.Replace(PROJECT_SRC_DIR=src_dir)

if env_name in HW_TEST_EXTRA_SRC:
    # フレームワーク内ライブラリの include パスを取得して CPPPATH に追加する。
    # env.Object() はノード作成時の CPPPATH をスナップショットするため、
    # LDF が後から追加するパスは届かない。ここで明示的に追加する。
    fw_dir = env.PioPlatform().get_package_dir("framework-arduinoadafruitnrf52")
    fw_lib_includes = [
        os.path.join(fw_dir, "libraries", "Bluefruit52Lib", "src"),
        os.path.join(fw_dir, "libraries", "Adafruit_TinyUSB_Arduino", "src"),
        os.path.join(fw_dir, "libraries", "Adafruit_LittleFS", "src"),
        os.path.join(fw_dir, "libraries", "Adafruit_nRFCrypto", "src"),
        os.path.join(fw_dir, "libraries", "InternalFileSytem", "src"),
        os.path.join(fw_dir, "libraries", "Wire"),
        os.path.join(fw_dir, "libraries", "SPI"),
        os.path.join(fw_dir, "cores", "nRF5"),
    ]
    env.Append(CPPPATH=fw_lib_includes)

    # ユーザーライブラリ（libdeps）の include パスも追加する
    libdeps_env_dir = os.path.join(env["PROJECT_DIR"], ".pio", "libdeps", env_name)
    if os.path.isdir(libdeps_env_dir):
        for lib_name in os.listdir(libdeps_env_dir):
            lib_path = os.path.join(libdeps_env_dir, lib_name)
            if os.path.isdir(lib_path):
                env.Append(CPPPATH=[lib_path])
                src_sub = os.path.join(lib_path, "src")
                if os.path.isdir(src_sub):
                    env.Append(CPPPATH=[src_sub])

    extra_objects = []
    for rel_file in HW_TEST_EXTRA_SRC[env_name]:
        cpp_file = os.path.join(env["PROJECT_DIR"], rel_file)
        obj = env.Object(
            target=os.path.join(
                env["BUILD_DIR"], "extra_src",
                rel_file.replace("/", "_").replace(".cpp", ".o")
            ),
            source=cpp_file,
        )
        extra_objects.append(obj)
    env.Append(PIOBUILDFILES=extra_objects)
