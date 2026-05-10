# テストディレクトリ構成と実行手順

**対象プロジェクト:** BLE + e-Paper + CO₂ モニター（Seeed XIAO nRF52840 Sense）  
**インタフェース仕様の正:** `../../2_basic_design/interface_spec.md`  
**設計書の正:** `../../3_detailed_design/`

---

## ディレクトリ構成

```
test/
├── README.md                         ← このファイル（AI エージェント向け全体ガイド）
├── test_eco2mapper/
│   └── test_eco2mapper.cpp           → pio test -e native_eco2mapper  [層1]
├── test_statemachine/
│   └── test_statemachine.cpp         → pio test -e native_statemachine [層1]
└── hw/                               ← 層2〜3: 実機書き込みテスト
    ├── sgp40_only/main.cpp           → pio run -e test_sgp40_only -t upload
    ├── epaper_only/main.cpp          → pio run -e test_epaper_only -t upload
    ├── imu_wake/main.cpp             → pio run -e test_imu_wake -t upload   [層2]
    ├── ble_adv/main.cpp              → pio run -e test_ble_adv -t upload     [層2]
    └── sleep_cycle/main.cpp          → pio run -e test_sleep_cycle -t upload [層3]
```

---

## テスト層の設計思想

この製品はスリープ時間が大半（約10分周期）のため、**ハードをそのままテストする手段が限られる**。
以下の3層で分担する。

| 層 | 実行環境 | 対象 | 制約 |
|----|----------|------|------|
| 1 | PC (native) | 純粋ロジック | ハード不要、即実行可 |
| 2 | 実機 + Serial | ハード単体確認 | USB 接続、スリープなし |
| 3 | 実機 + Serial | スリープ・ウェーク通し | RTC を 5s に短縮して検証 |

---

## 層1: Native Unit Test

### 前提

- PlatformIO native platform を使用（PC 上で実行）
- テストフレームワーク: Unity（PlatformIO 組み込み）
- ハード依存なし → CI にそのまま組み込める

### 実行方法

`pio` は `.venv/` にインストールされているため、以下のどちらかで呼ぶ（詳細は `../README.md` §Setup）。

```bash
cd /home/naots/Project_BLE_ePaper_Co2_moniter/4_implementation

# --- 方法A: venv を activate してから（ターミナルセッション中 pio だけで打てる）
source .venv/bin/activate
pio test -e native_eco2mapper
pio test -e native_statemachine

# --- 方法B: 毎回フルパス指定（PATH を汚さない）
.venv/bin/pio test -e native_eco2mapper
.venv/bin/pio test -e native_statemachine
```

### eco2mapper（`test/test_eco2mapper/test_eco2mapper.cpp`）

**検証する契約:** `interface_spec.md §3.5` — OP-ECO2-01（スナップショット生成）

| テスト名 | 入力 | 期待出力 |
|----------|------|----------|
| `test_voc0_gives_baseline_eco2` | vocIndex=0 | eco2Ppm=400 |
| `test_voc500_clamps_eco2` | vocIndex=500 | eco2Ppm=2000（上限） |
| `test_vent_ok` | vocIndex=100 | vent=Ok |
| `test_vent_mild` | vocIndex=150 | vent=Mild |
| `test_vent_high` | vocIndex=250 | vent=High |
| `test_trend_up` | prev=100, cur=120 | trend=Up |
| `test_trend_down` | prev=200, cur=180 | trend=Down |
| `test_trend_flat` | prev=100, cur=110 | trend=Flat |
| `test_no_prev_is_flat` | prev=nullptr | trend=Flat |

### statemachine（`test/test_statemachine/test_statemachine.cpp`）

**検証する契約:** `interface_spec.md §3.1` イベント E-APP-01/02/03 に対する遷移

| テスト名 | 遷移 |
|----------|------|
| `test_boot_cold_start` | BOOT + COLD_START → HW_INIT |
| `test_measure_ok` | RUN_MEASURE + MEASURE_OK → RUN_DISPLAY |
| `test_measure_err` | RUN_MEASURE + MEASURE_ERR → ERROR_LATCH |
| `test_ble_window_expired` | BLE_WINDOW + BLE_WINDOW_EXPIRED → POWER_DOWN |
| `test_unknown_events_no_change` | 不正イベントで状態変化しないこと |

---

## 層2: On-Device Smoke Test

### 前提

- 実機（XIAO nRF52840 Sense）を USB 接続
- `.venv/bin/pio run -e <env> -t upload && .venv/bin/pio device monitor -e <env>` で書き込み＋シリアル確認（activate 済みなら `pio` だけでよい）
- Serial の出力を目視判定（合否は行頭の `[OK]`/`[NG]` で識別）

### imu_wake（`test/hw/imu_wake/main.cpp`）

**検証する契約:** `interface_spec.md §3.3` — OP-IMU-01、E-IMU-01

- LSM6DS3 を I2C で初期化して "IMU OK" を出力
- WAKE_UP しきい値を設定し、実際に振動させると "WAKE_UP detected" が出ること
- INT1 ピン（`pin::kImuInt1`）の H/L 変化を Serial で確認

### ble_adv（`test/hw/ble_adv/main.cpp`）

**検証する契約:** `interface_spec.md §3.8` — OP-BLE-01〜03、D-BLE-01

- カスタムサービス（UUID: `11111101-...`）を登録して広告開始
- nRF Connect（スマホアプリ）で UUID・GATT Read 値を目視確認
- `Serial` に接続状態と各特性値の write を出力

---

## 層3: Sleep-Wake Cycle Test

### sleep_cycle（`test/hw/sleep_cycle/main.cpp`）

**検証する契約:** `interface_spec.md §3.2/3.3/3.7` — E-TMR-01、E-IMU-01、OP-SLP-01/02、E-SLP-01

**スリープの検証方法:**
本番（600s）では観測が困難なため、このテストは **RTC を 5秒に短縮**して 1サイクルを繰り返す。

- 起動後 Serial に `WakeReason` を出力
- SGP40 測定 → e-Paper 更新 → Serial 出力
- 5秒スリープ（RTC wake）→ 復帰し Serial に `wake from timer` を出力
- 本体を振動させると `wake from IMU` と出力されること

```
期待する Serial 出力例:
[wake] reason=ColdBoot
[meas] vocIndex=120 eco2=880ppm
[epd] refresh done
[sleep] entering WFE deep sleep (5s)
  --- 5秒後 ---
[wake] reason=Timer10Min
...
  --- 振動させた場合 ---
[wake] reason=ImuMotion
```

---

## 既存テスト（参考）

| env | 内容 | 確認項目 |
|-----|------|----------|
| `test_sgp40_only` | SGP40 単体 I2C | シリアル番号取得、SRAW_VOC 周期出力 |
| `test_epaper_only` | e-Paper 単体 SPI | 画面更新、黒白交互描画 |

---

## AI エージェント向けノート

- **Wire1 I2C バスリカバリ必須:** nRF52840 の TWIM ペリフェラルは、IMU パワーオン直後に SDA がスタックしている場合、最初の `endTransmission()` で無限ハングする。`Wire1.begin()` の前に SCL を 9 回手動トグルして bus recovery を実施すること。`test/hw/imu_wake/main.cpp` の `recoverI2cBus()` を参照。本番コードの IMU 初期化でも同じ処理が必要。
- **スリープ実装** は `src/power/SleepController.cpp` の `enterSystemOffOrDeepSleep()` で WFE ループを使用（System OFF ではない）。IMU が RTC と同時に wake source になれることが前提。
- **ピン定義** はすべて `include/app/pin_config.h` に集約。テストコードもこれを参照する。
- **GxEPD2 ライブラリに手動パッチあり**（SWRESET delay 10ms→200ms）。`pio run` 後に `.pio/libdeps` が再生成された場合は再適用が必要。パッチ詳細は `../../` 配下の memory を参照。
- `MeasurementFrame` 構造体は `include/app/types.h` に定義。テスト内で直接使用可。
- `Eco2Mapper` と `StateMachine` はハード依存なし → native テストで完結できる。

### PlatformIO 6.1 native テストの落とし穴（実機調査済み）

新たに native テストを追加するときは以下の3点を守ること。

| 制約 | 正しい設定 | 誤り例 |
|------|-----------|--------|
| テストスイートのディレクトリ名は `test_` で始める | `test/test_eco2mapper/` | `test/eco2mapper/` |
| src/ を test ビルドに含めるには明示が必要 | `test_build_src = yes` | （未設定）→ リンクエラー |
| src/ の絞り込みは除外から書く | `build_src_filter = -<*> +<sensor/Eco2Mapper.cpp>` | `+<sensor/Eco2Mapper.cpp>` のみ |
