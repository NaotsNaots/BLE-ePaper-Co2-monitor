# デバイス・チップ公式ドキュメント索引

`1_requirements/hardware_spec.md` に記載されている部材について、**メーカー製品規格・評価用 Wiki・実装ライブラリ**を一通り並べています。**リンク切れや版変更**は定期的に確認してください。

---

## クイック索引（URL）

| 区分 | 部材／資料 | URL（正式・直接） |
|------|-------------|-------------------|
| マイコン基板 | **Seeed Studio XIAO nRF52840 Sense** — 開発ガイド | [Getting Started (XIAO BLE シリーズ)](https://wiki.seeedstudio.com/XIAO_BLE/) |
| ↑ | PlatformIO と XIAO nRF52840-Sense | [XIAO nRF52840-Sense with PlatformIO](https://wiki.seeedstudio.com/xiao_nrf52840_with_platform_io/) |
| ↑ | ピン・アウト類（ショート一覧） | [XIAO シリーズ紹介（オープンマテリアル）](https://wiki.seeedstudio.com/SeeedStudio_XIAO_Series_Introduction) → **Sense** の schematic / KiCAD / pinout XLS等 |
| ↑ | IMU（LSM6DS3TR-C）利用例（Seeed Wiki） | [IMU Usage for XIAO nRF52840 Sense](https://wiki.seeedstudio.com/XIAO-BLE-Sense-IMU-Usage/) |
| SoC（チップ規格） | **Nordic nRF52840** — Product Spec（PDF は版更新される） | 製品ページ: [nRF52840](https://www.nordicsemi.com/Products/nRF52840) / **PDF 例**: [nRF52840 PS v1.9](https://docs.nordicsemi.com/bundle/nRF52840_PS_v1.9/resource/nRF52840_PS_v1.9.pdf) |
| ↑ | Nordic 開発ドキュメント集合 | [Infocenter](https://infocenter.nordicsemi.com)（検索：`nRF52840`／低消費電力は *POWER*／USB／SPI／TWI を参照） |
| IMU | **STMicro LSM6DS3TR-C** — 製品 | [製品ページ](https://www.st.com/en/mems-and-sensors/lsm6ds3tr-c.html) |
| ↑ | datasheet（PDF・主要） | [lsm6ds3tr-c.pdf](https://www.st.com/resource/en/datasheet/lsm6ds3tr-c.pdf) |
| VOC センサ | **Sensirion SGP40** — 製品 | [SGP40 カタログページ](https://sensirion.com/products/catalog/SGP40) |
| ↑ | datasheet（PDF・主要） | [Sensirion_Gas_Sensors_Datasheet_SGP40.pdf](https://sensirion.com/media/documents/296373BB/6203C5DF/Sensirion_Gas_Sensors_Datasheet_SGP40.pdf)（`I²C`=0x59、コマンド表、ヒータ無効など） |
| ↑ | VOC Index / SRAW 処理（公式ソース） | C 共通: [Sensirion/gas-index-algorithm](https://github.com/Sensirion/gas-index-algorithm)；Arduino 向けラッパ: [Sensirion/arduino-gas-index-algorithm](https://github.com/Sensirion/arduino-gas-index-algorithm) |
| ↑ | I²C ドライバ（Arduino） | [Sensirion/arduino-i2c-sgp40](https://github.com/Sensirion/arduino-i2c-sgp40)（依存: **Sensirion Core**） |
| 昇圧 DC-DC | **TOREX XCL102** — シリーズ紹介 | [XCL102 | TOREX](https://product.torexsemi.com/en/series/xcl102) |
| ↑ | datasheet（PDF） | [xcl102.pdf](https://product.torexsemi.com/system/files/series/xcl102.pdf)（Vin 範囲、CE、外部コンデンサ、保護回路等） |
| ショットキ | **11EQS03L** | 型番は **NIEC（日本インターエレクトロニクス）** 系列と **Kyocera Kinseki（KSS）** 等で文献が分かれることがある。**購入ロットのメーカー表の PDF** を正とする。参考: [ALLDATASHEET (NIEC)](https://www.alldatasheet.com/datasheet-pdf/pdf/92114/NIEC/11EQS03L.html) / [ALLDATASHEET (KSS)](https://www.alldatasheet.com/datasheet-pdf/pdf/1052022/KSS/11EQS03L.html) |
| e-Paper | **Waveshare 2.9inch e-Paper Module** | [Manual（`#C`）](https://www.waveshare.com/wiki/2.9inch_e-Paper_Module_Manual#C)；[製品ページ](https://www.waveshare.com/product/2.9inch-e-paper-module.htm) |
| ↑ | 改版 **(C)** の別マニュアル | [2.9inch e-Paper Module (C) Manual](https://www.waveshare.com/wiki/2.9inch_e-Paper_Module_%28C%29_Manual) |
| 表示ライブラリ | **GxEPD2**（Waveshare 等 SPI パネル） | [ZinggJM/GxEPD2](https://github.com/ZinggJM/GxEPD2)（README に 2.9" 系ドライバクラス一覧。例: **GDEM029T94** / **GxEPD2_290_T94_V2** 等 — **実板の FPC／Waveshare Rev** と突合） |
| FW スタック | **Adafruit nRF52 Arduino Core**（要件で Arduino + Adafruit） | [Adafruit_nRF52_Arduino](https://github.com/adafruit/Adafruit_nRF52_Arduino)（ボード定義・ブートローダー・低消費 API の手掛かり） |
| ビルド | **PlatformIO** | [platformio.org](https://platformio.org)；Seeed ボード向け例は上記 Seeed Wiki |

---

## カテゴリ別メモ（設計でよく閲覧する章）

### 1. XIAO nRF52840 Sense × nRF52840

- **Seeed Wiki**: 電源、USB、バッテリ充電（BQ25101）、GPIO／ADC 数、**Sense 専用の IMU・PDM** の有無。
- **nRF52840 PS**: 供給電圧範囲、**SYSTEM OFF / 各種 sleep**、**SPI master**、**TWIM (I²C)**、**USB**、**ADC**、最大レート。
- 省電要求（10 分周期・IMU ウェーク）は **PS の Power 章** と **Adafruit core の sleep 系 API** を対応付けて詳細設計へ落とす。

### 2. SGP40

- **Datasheet**: I²C アドレス **0x59**、測定コマンド、測定時間、**`sgp4x_turn_heater_off`**（省電）、自己診断。
- **Gas Index Algorithm**: デフォルト **1 s** 間隔推奨、**10 s** も言及あり（要件の 10 分周期とは別 — **起動時だけ連続サンプル**するか、**スリープ前に 1 点**かはアルゴリズム設計で決定）。低電力例は **arduino-gas-index-algorithm** リポジトリを参照。

### 3. LSM6DS3TR-C

- **Datasheet**: 割り込みピン、**WAKE_UP**、**double-click** 等、レジスタマップ、I²C アドレス（通常 0x6A/0x6B 系 — **XIAO 上の配線は回路図で確認**）。
- Seeed Wiki は **Seeed nrf52 mbed** 推奨の記述あり — 本プロジェクトは **Adafruit core** 前提なので、**Arduino 用 ST 公式／コミュニティドライバ**との整合を取る。

### 4. XCL102

- **PDF**: 入力レンジ、**起動電圧**、外部 **Cin/Cout**、**CE**（スタンバイ）、突入電流と **100µF** の役割の根拠づくり。
- 品番（**XCL102D333CR-G** 等）は実機の **3.3V 固定**に合わせて選定。

### 5. 11EQS03L

- **PDF（入手元をロットで確定）**: **\(V_\mathrm{f}\)–\(I_\mathrm{f}\)** 曲線、逆耐圧、パッケージ熱。
- 本プロジェクトの **実効 2.9V レール**は、ここ＋ XCL102 調整＋負荷電流で最終確認。

### 6. Waveshare 2.9" e-Paper

- **Wiki**: 解像度 **296×128**、**SPI mode 0**、更新時間、消費電流、**5V/3.3V** の注意書き。
- **GxEPD2**: パネル世代（V1/V2、**(C)**、FPC 記号）と **ドライバクラス**の対応を **実物ラベル**で確定。`init()` の **「clever reset」** オプションは Waveshare 基板で必要になることがある（ライブラリ example・コメント参照）。

---

## 注意（索引に含めないもの）

- **単三電池ボックス**、**100µF/35V 電解**: 一般部品 — メーカー各社の **温度・リップル電流**を定格で確認すれば足りる。
- **GPIO／ピンアサイン**: [`hardware_spec.md`](../hardware_spec.md)「GPIO／ピンアサイン」を正とする。

---

関連: [../hardware_spec.md](../hardware_spec.md)、[../architecture.md](../architecture.md)
