# シーケンス図

**フェーズ:** 詳細設計  
**根拠:** [`block_diagram.md`](../2_basic_design/block_diagram.md)、[`interface_spec.md`](../2_basic_design/interface_spec.md)、[`architecture.md`](../1_requirements/architecture.md) BLE 表、[ユースケース補助 `use_case_diagram.md`](../1_requirements/use_case_diagram.md)。

オブジェクト名は [`class_diagram.md`](class_diagram.md) のクラスに一致させる。**公式ライブラリ**は構造図と同じ実名で記載する。

---

## シナリオ A — 定期ウェーク（F2のみ、BLE なし）

**前提:** 約10分 RTC 復帰。IMU はバックグラウンドで待ち受けているが **当サイクルはタイマ主導**。

```mermaid
sequenceDiagram
  autonumber
  participant RTC as nRF RTC / 比較_WAKEUP
  participant App as ApplicationController
  participant Slp as SleepController
  participant SGP as Sgp40Pipeline
  participant DEV as SensirionI2CSgp40
  participant VOC as VOCGasIndexAlgorithm
  participant MAP as Eco2Mapper
  participant EPD as EpdPresenter
  participant GX as GxEPD2_3C

  RTC->>App: CPU 復帰 (WakeReason=Timer)
  App->>SGP: measureVocIndex(sraw, vocIdx)
  SGP->>DEV: measureRawSignal(0x8000, 0x6666, srawVoc)
  DEV-->>SGP: err==0, srawVoc
  SGP->>VOC: process((int32_t)srawVoc)
  VOC-->>SGP: vocIndex (int32_t)
  SGP-->>App: OK, sraw, vocIndex

  App->>MAP: snapshotFromVoc(vocIndex, prev, frame)
  MAP-->>App: MeasurementFrame 確定 (index++, eco2, vent, trend)

  App->>EPD: show(frame)
  EPD->>GX: init / setFullWindow / print / display (first/next loop)
  GX-->>EPD: 描画完了
  EPD->>GX: hibernate()
  Note over App: WakeReason!=Imu → BLE スキップ (OP-APP-04)

  App->>SGP: heaterOffForSleep()
  SGP->>DEV: turnHeaterOff()

  App->>Slp: armRtcWakeupSeconds(600); configureGpioSense(IMU_PIN)
  App->>Slp: enterSystemOffOrDeepSleep()
```

---

## シナリオ B — IMU ウェーク（F6：測定→表示→BLE 60秒→スリープ）

**前提:** `ImuWakeSource` が LSM6DS3 の WAKE_UP を **nRF sense ピン**へ届け済み。復帰後 `WakeReason=ImuMotion`。

```mermaid
sequenceDiagram
  autonumber
  participant IMU as LSM6DS3_INT1
  participant App as ApplicationController
  participant SGP as Sgp40Pipeline
  participant DEV as SensirionI2CSgp40
  participant VOC as VOCGasIndexAlgorithm
  participant MAP as Eco2Mapper
  participant EPD as EpdPresenter
  participant BLE as EnvBleService
  participant AD as BluefruitAdvertising
  participant SD as SoftDevice_S132
  participant PH as BLE_Central_phone

  IMU->>App: GPIO 復帰 / 割込フラグ (WakeReason=Imu)

  App->>SGP: measureVocIndex(...)
  SGP->>DEV: measureRawSignal(...)
  DEV-->>SGP: sraw
  SGP->>VOC: process(sraw)
  VOC-->>SGP: vocIndex

  App->>MAP: snapshotFromVoc(...)
  MAP-->>App: MeasurementFrame

  App->>EPD: show(frame)

  App->>BLE: updateGattSnapshot(frame), chr.write LE
  App->>BLE: startAdvertising(60000 ms)
  BLE->>AD: addFlags + addService + start(connectable)
  AD->>SD: 広告 PDU
  SD-->>PH: ADV_IND (connectable)

  loop 最大 60 秒
    PH->>SD: 接続要求
    SD->>BLE: 接続イベント
    PH->>SD: GATT Read UUID ...1101 / 1102 / ...
    BLE-->>PH: uint16 / uint32 LE 直列化値
    opt CCCD 購読時
      BLE->>PH: Notify (...1103 sampleIndex 等)
    end
  end

  App->>BLE: stopAdvertising()
  BLE->>AD: stop()

  App->>SGP: heaterOffForSleep()
  App->>EPD: hibernatePanel() (再確認)
  App->>App: SleepController: arm + enterSystemOffOrDeepSleep
```

**GATT 値の直列化（実装契約）**

| 特性 UUID（末尾） | `BLECharacteristic::write` 内容 |
|-------------------|----------------------------------|
| `...1101` | `(uint16_t)vocIndex` — Sensirion 出力を uint16 に収める（クランプ仕様は `Eco2Mapper` 側で文書化） |
| `...1102` | `(uint16_t)frame.eco2Ppm` |
| `...1103` | `(uint32_t)frame.sampleIndex` LE |
| `...1104` | 固定長 ASCII／セマンティック版（例：`"0.1.0\\0"`）— `sizeof` 固定 |

---

## シナリオ C — 初回起動とセルフテスト（開発／製造オプション）

**目的:** [`SensirionI2CSgp40::executeSelfTest`](https://github.com/Sensirion/arduino-i2c-sgp40) を **S1 `HW_INIT`** で任意実行し、`0xD400` 以外なら S7。

```mermaid
sequenceDiagram
  participant App as ApplicationController
  participant SGP as Sgp40Pipeline
  participant DEV as SensirionI2CSgp40

  App->>SGP: begin(Wire)
  SGP->>DEV: begin(Wire)
  opt 工場／デバッグビルド
    App->>DEV: executeSelfTest(result)
    DEV-->>App: result (期待 0xD400)
  end
```

---

## シナリオ D — F7 低電池（回路実装後）

将来：`BatteryMonitor::isLow()` が true のとき **S6** へ分岐し、`EpdPresenter` に Low Battery 専用画面。以降 `SleepController` で極浅いポーリングのみ、または停止。

```mermaid
sequenceDiagram
  participant App as ApplicationController
  participant BAT as BatteryMonitor
  participant EPD as EpdPresenter

  App->>BAT: isLow()
  BAT-->>App: true
  App->>EPD: showLowBattery()
  Note over App: 以降は安全停止ポリシー（[`features.md`](../1_requirements/features.md) F7）
```

---

## Mermaid 記法・ツールについて

- GitHub／多くの VS Code 拡張で **`sequenceDiagram`** を描画可能。
- 本ドキュメントでは `participant GX as GxEPD2_BW` のように、C++ テンプレート引数は書かず短縮している（レンダラ互換のため）。実体は `class_diagram.md` の GxEPD2 節に従う。

---

## トレーサビリティ

| シナリオ | 機能 ID / ユースケース |
|----------|------------------------|
| A | F2, F3, F4, F5 |
| B | F3, F4, F6（Read／Notify／Write なし） |
| C | F2 初期化品質 |
| D | F7 |
