# IF仕様書（ソフトウェア・インタフェース仕様書）

**フェーズ：** 基本設計  
**目的：** [`block_diagram.md`](block_diagram.md) で識別した **ソフトモジュール境界** における **論理 I/F（オペレーション・イベント・論理データ）** を契約として固定する。詳細設計ではクラス・API・状態機械が **本書の契約を実現する形** で具体化する。

**電気／ピンの正：** [`../1_requirements/hardware_spec.md`](../1_requirements/hardware_spec.md)。  
**BLE GATT の UUID／値型（ドラフト）：** [`../1_requirements/architecture.md`](../1_requirements/architecture.md)。正式 UUID・型は基本設計レビューで確定し、`architecture.md` と本書を同期する。

**更新ルール：** ブロック構成または公式スタックの選定が変わったら、本書の該当節と §6 トレーサビリティを追従する。

---

## 1. I/F の記述フォーマット（本プロジェクトでの約束）

**インタフェース仕様とは：** モジュール境界で「何を渡すか」「何が起きるか」を **契約として固定**したものです。詳細設計でのクラス設計は、その契約を満たす **実装手段** にすぎません。

長い説明文を **1つの表セルに書き並べない** でください。読み手が項目を拾えません。各モジュール（§3）は次の **見出し構成** を守ります。

| 見出し | 書くこと |
|--------|----------|
| **概要** | 提供者・利用者・関連 F-ID／リンクのみ（短文） |
| **境界契約** | 下記 4 区分を **別々の表または箇条書き** で書く |
| **前置・制約** | 順序・電源・配線制約など（箇条書き） |
| **実装方針（公式資産）** | 公式ライブラリ名と「詳細設計で名前が決まる」旨 |

**境界契約の 4 区分（ここが「IF の説明」の本体です）：**

| 区分 | 意味 |
|------|------|
| **入力** | このモジュールが **境界をまたいで受け取る** 論理データ、または上位からの前提・設定値 |
| **出力** | このモジュールが **境界をまたいで返す／公開する** 論理データ・状態 |
| **オペレーション** | 上位が **明示的に要求できる処理**（同期 API に相当する契約） |
| **イベント** | **非同期に発生しうる通知**（ウェーク、タイムアウト、割込み相当）。オペレーションとは別表または別リストで書く |

未確定は **TBD** とし、型・単位はわかる範囲で表に列として載せる。

**詳細設計への負わせるもの：** C++ クラス名・メソッド名、FreeRTOS の有無、シングルトン構成など。

---

## 2. 公式ライブラリ・資料との対応（ハード境界の実装指針）

要件 [`architecture.md`](../1_requirements/architecture.md) および索引 [`../1_requirements/hardware_documentation/README.md`](../1_requirements/hardware_documentation/README.md) に基づき、**各モジュールは可能な限り次をそのまま活用**する。境界 I/F は「ライブラリの上に薄いラッパを置いてもよいが、**論理データとライフサイクルは本書で固定**する」ことを前提とする。

| 論理モジュール | 推奨スタック／資料 | 備考 |
|----------------|-------------------|------|
| ビルド・MCU | **PlatformIO**、**Arduino Framework**、**Adafruit nRF52 Board Support** | [Seeed: XIAO nRF52840-Sense with PlatformIO](https://wiki.seeedstudio.com/xiao_nrf52840_with_platform_io/)、[Adafruit_nRF52_Arduino](https://github.com/adafruit/Adafruit_nRF52_Arduino) |
| SGP40 I²C | **Sensirion/arduino-i2c-sgp40**（依存 **Sensirion Core**） | [arduino-i2c-sgp40](https://github.com/Sensirion/arduino-i2c-sgp40)；データシートで I²C **0x59** 等 |
| VOC Index 処理 | **Sensirion/gas-index-algorithm**／**arduino-gas-index-algorithm** | ドライバと組み合わせて Index 取得；サンプル間隔はアルゴリズム README・要件 F2／F3 と整合 |
| e-Paper | **GxEPD2**（ZinggJM） | [GxEPD2](https://github.com/ZinggJM/GxEPD2)；実板 Rev／FPC と **ドライバクラス**を突合（`hardware_spec`・Waveshare Wiki） |
| IMU（ウェーク） | **LSM6DS3TR-C**（データシート）＋ Adafruit／Arduino 実装の選択は詳細設計 | [Seeed: IMU Usage](https://wiki.seeedstudio.com/XIAO-BLE-Sense-IMU-Usage/)、[ST datasheet](https://www.st.com/resource/en/datasheet/lsm6ds3tr-c.pdf)；Sense 載せ替え回路で INT→nRF ピンを確定 |
| BLE | **Adafruit BLE API**／SoftDevice 前提の Arduino 実装 | カスタムサービス・Read／Notify；Write なし（要件 F6） |
| 省電 | **nRF52840** POWER／SYSTEM OFF 系 **＋** core 付属の sleep 系 API | Nordic [Infocenter](https://infocenter.nordicsemi.com)、PS PDF；IMU 継続動作との両立は `hardware_spec`「IMU とスリープ」 |

---

## 3. モジュール境界 I/F

以下、各ブロックは §1 の **境界契約（入力／出力／オペレーション／イベント）** で読む。

### 3.1 アプリ／スケジューラ（中枢）

**概要：** **提供者** アプリ／スケジューラ。**利用者（協調ブロック）** 計時ウェーク、IMU、SGP40 ドライバ、e-Paper ドライバ、省電／スリープ、BLE、（将来）電池監視。**関連** F2〜F6、F7（将来）、[`features.md`](../1_requirements/features.md) §3。

#### 境界契約（インタフェース）

本ブロックは **調停役** のため、「外部から見た公開 IF」より **下位への要求とイベント統合** を契約として書く。

**入力（イベント／状態・このブロックが反応するもの）**

| ID | 論理名 | 意味 |
|----|--------|------|
| E-APP-01 | 計時ウェーク | 約10分周期で処理サイクルを開始するトリガ |
| E-APP-02 | IMU ウェーク | 振動検知による復帰。処理後は BLE 広告許可 |
| E-APP-03 | BLE ウィンドウ終了 | 60秒経過によりラジオ停止・スリープ準備へ |
| E-APP-04 | （将来）低電池 | しきい超過時の表示・安全停止シーケンス |

**出力（下位モジュールへの論理データ・保持データの契約）**

| ID | 論理名 | 意味・備考 |
|----|--------|------------|
| D-APP-01 | 当サイクル VOC Index | SGP40／ガスインデックス経由で確定した値 |
| D-APP-02 | 当サイクル eCO₂ | §3.5 の出力をそのまま表示・BLE に渡す前提 |
| D-APP-03 | 換気・アイコン区分 | §3.5 の出力 |
| D-APP-04 | トレンド区分 | §3.5 の出力 |
| D-APP-05 | サンプルインデックス | **単調増加**。RAM／Flash 保持は詳細設計 |

**オペレーション（このブロックが序序で実行する処理の論理名）**

| ID | 論理オペレーション | 内容 |
|----|-------------------|------|
| OP-APP-01 | 測定パイプライン実行 | SGP40 ドライバ経由で RAW／VOC Index を取得し §3.5 に渡す |
| OP-APP-02 | 表示更新要求 | §3.6 に描画用論理データを渡す |
| OP-APP-03 | スリープ移行要求 | §3.7 に遷移。定期サイクル終了後および BLE 終了後 |
| OP-APP-04 | BLE ウィンドウ制御 | IMU ウェーク時のみ §3.8 で広告開始／60秒後停止 |

**イベント（発生源→アプリ）**

- 計時ウェーク ← §3.2  
- IMU ウェーク ← §3.3  
- BLE 接続／切断／タイムアウト ← §3.8  

#### 前置・制約

- IMU ウェーク時は **測定 → 表示 → BLE 最大60秒** の順（[`block_diagram.md`](block_diagram.md) 図2）。
- 定期ウェークのみのときは **BLE を開始しない**。
- GPIO／ピンは `hardware_spec`「GPIO／ピンアサイン」に従う。

#### 実装方針

単一 `loop`／RTOS は詳細設計。**状態機械**は §6 トレーサビリティと整合させる。

---

### 3.2 計時ウェーク（F2）

**概要：** **提供者** 計時ウェーク論理ブロック。**利用者** アプリ。**関連** F2、[Infocenter POWER](https://infocenter.nordicsemi.com)。

#### 境界契約（インタフェース）

**入力**

| ID | 論理名 | 意味 |
|----|--------|------|
| I-TMR-01 | 次回ウェーク設定 | スリープ前にアプリが arm（時刻／カウンタ等 — 詳細設計で API 化） |

**出力**

| ID | 論理名 | 意味 |
|----|--------|------|
| （任意） | 内部カウンタ | TBD。デバッグ用に読む場合のみ |

**オペレーション**

| ID | 論理オペレーション | 内容 |
|----|-------------------|------|
| OP-TMR-01 | アーム／再設定 | 省電復帰後の次の約10分ウェークを予約 |

**イベント**

| ID | 論理名 | 内容 |
|----|--------|------|
| E-TMR-01 | 計時ウェーク発火 | 約10分周期。ジッターは実装手段依存 |

#### 前置・制約

- RTC／LFCLK／WDT／ソフトカウンタ等の **選定は基本設計で確定**（[`features.md`](../1_requirements/features.md) F2 脚注）。
- 省電モードとの両立は Nordic／Adafruit 資料で検証（詳細設計）。

#### 実装方針

**Adafruit nRF52 core** 前提でタイマ・低消費 API を選択。**mbed 推奨記述は本プロジェクト対象外**。

---

### 3.3 IMU／ウェーク検知（内蔵 LSM6DS3、F6）

**概要：** **提供者** IMU（ハード＋検知設定／ISR 論理）。**利用者** アプリ。**関連** F6、[LSM6DS3TR-C datasheet](https://www.st.com/resource/en/datasheet/lsm6ds3tr-c.pdf)、[Seeed IMU Usage](https://wiki.seeedstudio.com/XIAO-BLE-Sense-IMU-Usage/)。

#### 境界契約（インタフェース）

**入力**

| ID | 論理名 | 意味 |
|----|--------|------|
| I-IMU-01 | 検知設定 | モード・しきい・フィルタ（適応変更可 — [`features.md`](../1_requirements/features.md) §3） |
| I-IMU-02 | 割込みクリア要求 | ウェーク後の確認／オーバーラン回避 |

**出力**

| ID | 論理名 | 意味 |
|----|--------|------|
| D-IMU-01 | ウェーク原因メタデータ | 「IMU 由来」をアプリが定期ウェークと区別するために利用（詳細設計で表現） |

**オペレーション**

| ID | 論理オペレーション | 内容 |
|----|-------------------|------|
| OP-IMU-01 | 検知構成の適用 | レジスタ／ライブラリ設定を反映 |

**イベント**

| ID | 論理名 | 内容 |
|----|--------|------|
| E-IMU-01 | 運動ウェーク検知 | 振動等。INT→MCU 復帰 |

#### 前置・制約

- [`hardware_spec.md`](../1_requirements/hardware_spec.md)「IMU とスリープ」どおり INT→GPIO の経路が必要。
- SoftDevice／sleep の組み合わせで復帰失敗があり得るため **復帰検証を詳細設計ゲートとする**。

#### 実装方針

ST **WAKE_UP** 等＋ Seeed Wiki。**Arduino ドライバ**は Adafruit core と競合しないものを選定（詳細設計）。

---

### 3.4 SGP40 ドライバ層（Sensirion I²C）

**概要：** **提供者** SGP40 ドライバ。**利用者** アプリ；ガスインデックス処理。**関連** F2、F3、[SGP40 datasheet](https://sensirion.com/media/documents/296373BB/6203C5DF/Sensirion_Gas_Sensors_Datasheet_SGP40.pdf)、[`hardware_documentation`](../1_requirements/hardware_documentation/README.md)。

#### 境界契約（インタフェース）

**入力**

| ID | 論理名 | 意味 |
|----|--------|------|
| I-SGP-01 | 測定トリガ | アプリからの単発／連続開始指示 |

**出力**

| ID | 論理名 | 意味・備考 |
|----|--------|------------|
| D-SGP-01 | RAW（SRAW） | Sensirion 仕様の生値 |
| D-SGP-02 | VOC Index | ガスインデックスアルゴリズム適用後（単位・レンジは Sensirion） |
| D-SGP-03 | エラー種別 | I²C NACK、チェックサム異常等 — 詳細設計で列挙 |

**オペレーション**

| ID | 論理オペレーション | 内容 |
|----|-------------------|------|
| OP-SGP-01 | 初期化 | センサ起動・設定 |
| OP-SGP-02 | 測定実行 | 単発または連続フレーム |
| OP-SGP-03 | 省電コマンド | 例：ヒータオフ（データシート／ライブラリに従う） |

**イベント**

- （同期 API のみなら省略可。連続測定時は「サンプル準備完了」コールバックを詳細設計で追加してよい）

#### 前置・制約

- I²C は `hardware_spec` の **確定ピン**後に検証。
- アルゴリズム推奨間隔と約10分間欠のギャップは、アプリ側でウォームアップ等により埋める — **手順は詳細設計**。

#### 実装方針

**[Sensirion/arduino-i2c-sgp40](https://github.com/Sensirion/arduino-i2c-sgp40)** ＋ **[arduino-gas-index-algorithm](https://github.com/Sensirion/arduino-gas-index-algorithm)** を既定とする。

---

### 3.5 VOC Index／eCO₂ 推計（論理ブロック F3）

**概要：** **提供者** eCO₂ 推計（アプリ内モジュールまたは独立コンポーネント）。**利用者** アプリ本体、e-Paper 表示入力、BLE 特性値。**関連** F3、[gas-index-algorithm](https://github.com/Sensirion/gas-index-algorithm)。

ここでの **IF** は「Sensirion が返す VOC Index」を **ユーザー表示・無線公開に耐える 1 サイクル分のスナップショット** に変換する **境界契約**である（推計式そのものは詳細設計／コード）。

#### 境界契約（インタフェース）

**入力**

| ID | 論理名 | 型・単位（暫定） | 意味 |
|----|--------|------------------|------|
| I-ECO2-01 | VOC Index（当サイクル） | Sensirion 定義（整数レンジはライブラリ仕様に従う） | §3.4 が確定させた値 |
| I-ECO2-02 | （任意）短期履歴 | TBD（例：直近 N 件の Index） | 推計アルゴリズムが必要なら追加 |
| I-ECO2-03 | （任意）内部状態 | TBD | 再起動跨ぎ・平滑化バッファ等 |

**出力**

| ID | 論理名 | 型・単位（暫定） | 意味 |
|----|--------|------------------|------|
| D-ECO2-01 | eCO₂ 推計値 | uint16 相当（実スケールは表示・[`architecture.md`](../1_requirements/architecture.md) BLE と整合） | **真 CO₂ ではない**（要件どおり） |
| D-ECO2-02 | 換気必要性区分 | 列挙（詳細設計で値定義） | F4 の換気アイコンに対応 |
| D-ECO2-03 | トレンド区分 | 列挙：`up`／`down`／`flat`（詳細設計で命名確定） | **直前に確定したスナップショット**との比較 |

**オペレーション**

| ID | 論理オペレーション | 内容 |
|----|-------------------|------|
| OP-ECO2-01 | スナップショット生成 | 上記入力を受け、**今サイクル分の出力一式**を確定する（表示・BLE が参照する単位はこれ） |

**イベント**

- 特になし（同期処理でよい）。将来ストリーム化する場合は詳細設計で追加。

#### 前置・制約

- **真 CO₂ は非測定**。ユーザー向け脚注文言は製品側検討。
- 係数・式は実装保有；本書では **入出力ペアと OP-ECO2-01 の意味**のみ固定する。

#### 実装方針

**VOC Index の算出経路は Sensirion 公式アルゴリズムを優先**。**eCO₂ と換気・トレンドは製品固有**として §3.5 の出力契約を満たす。

---

### 3.6 e-Paper ドライバ層（GxEPD2、F4）

**概要：** **提供者** e-Paper ドライバ。**利用者** アプリ。**関連** F4、[Waveshare 2.9" Manual](https://www.waveshare.com/wiki/2.9inch_e-Paper_Module_Manual#C)。

#### 境界契約（インタフェース）

**入力**

| ID | 論理名 | 意味 |
|----|--------|------|
| I-EPD-01 | VOC Index | §3.5 または同等の確定値 |
| I-EPD-02 | eCO₂ | §3.5 の D-ECO2-01 |
| I-EPD-03 | サンプルインデックス | 画面に番号を出す場合など（任意） |
| I-EPD-04 | 換気区分／トレンド区分 | アイコン選択に利用 |

**出力**

| ID | 論理名 | 意味 |
|----|--------|------|
| — | （ユーザー視認） | システム境界外。論理データとしては「描画結果」のみ |

**オペレーション**

| ID | 論理オペレーション | 内容 |
|----|-------------------|------|
| OP-EPD-01 | 初期化 | パネル・バス設定 |
| OP-EPD-02 | 画面更新 | フルまたはパーシャル |
| OP-EPD-03 | パネル省電 | deep sleep 相当（GxEPD2／Wiki に従う） |

**イベント**

- 特になし（BUSY はドライバ内部）。必要なら詳細設計で追加。

#### 前置・制約

- SPI **mode 0**、**296×128**（[`hardware_spec.md`](../1_requirements/hardware_spec.md)）。
- 改版 **(C)** と **GxEPD2 のドライバクラス**は実機ラベルで突合。

#### 実装方針

**[GxEPD2](https://github.com/ZinggJM/GxEPD2)** をそのまま利用。RST／BUSY 等は `hardware_spec` の D 番号を `pin_config.h` 等でマクロ化する。

---

### 3.7 省電／スリープ制御（F5）

**概要：** **提供者** 省電／スリープ制御。**利用者** アプリ。**関連** F5、F6（ラジオ通常オフ）、[Adafruit_nRF52_Arduino](https://github.com/adafruit/Adafruit_nRF52_Arduino)。

#### 境界契約（インタフェース）

**入力**

| ID | 論理名 | 意味 |
|----|--------|------|
| I-SLP-01 | 遷移要求 | Deep Sleep／System OFF 等への指示 |
| I-SLP-02 | ウェークソース設定 | タイマ／GPIO（IMU INT） |

**出力**

| ID | 論理名 | 意味 |
|----|--------|------|
| D-SLP-01 | 現在の低消費状態 | TBD（デバッグ・自己診断用） |

**オペレーション**

| ID | 論理オペレーション | 内容 |
|----|-------------------|------|
| OP-SLP-01 | スリープ実行 | MCU／周辺の停止手順 |
| OP-SLP-02 | ウェークソース構成 | 復帰条件の登録 |

**イベント**

| ID | 論理名 | 内容 |
|----|--------|------|
| E-SLP-01 | 復帰完了 | CPU 再実行開始（詳細設計でマスク可否） |

#### 前置・制約

- IMU が待機中も検知できる **MCU／IMU 電源モードの両立**（[`hardware_spec.md`](../1_requirements/hardware_spec.md)）。
- **BLE は通常オフ**。F6 のみ最大 60秒。

#### 実装方針

Adafruit sleep API ＋ Nordic PS。**復帰確認**をゲートとする。

---

### 3.8 BLE スタック（F6）

**概要：** **提供者** BLE スタック。**利用者** アプリ、境界外 BLE Central。**関連** F6、[`architecture.md`](../1_requirements/architecture.md)。

#### 境界契約（インタフェース）— アプリ ↔ BLE

**入力**

| ID | 論理名 | 意味 |
|----|--------|------|
| I-BLE-01 | 公開スナップショット | VOC Index、eCO₂、サンプルインデックス、ファーム識別（型・LE は `architecture.md` ドラフト） |
| I-BLE-02 | ウィンドウ制御 | 広告開始／強制停止 |

**出力**

| ID | 論理名 | 意味 |
|----|--------|------|
| D-BLE-01 | 接続状態 | 詳細設計で列挙 |

**オペレーション**

| ID | 論理オペレーション | 内容 |
|----|-------------------|------|
| OP-BLE-01 | アドバタイズ開始／停止 | **connectable** |
| OP-BLE-02 | GATT サーバ登録 | カスタムサービス |
| OP-BLE-03 | Read 応答データ更新 | Central Read に応じる現在値 |
| OP-BLE-04 | Notify 送信 | CCCD 購読時のみ |

**イベント**

| ID | 論理名 | 内容 |
|----|--------|------|
| E-BLE-01 | Central 接続／切断 | アプリがログやタイマと協調する場合に利用 |
| E-BLE-02 | 60秒ウィンドウ満了 | ラジオ停止トリガ |

#### 境界契約 — BLE Central ↔ 装置（システム境界）

| 種別 | 契約 |
|------|------|
| Read | 各特性の現在値を返す |
| Notify | CCCD 購読時に送信可（項目は最小ポートフォリオで可） |
| Write | **なし** |
| セキュリティ | Just Works／オープン Read でよい（[`features.md`](../1_requirements/features.md)） |

#### 前置・制約

- **広告は IMU ウェーク後のみ**、最大 **60秒**。終了後スリープへ。
- 時刻解釈は **Central 側**。装置はインデックス等のみ。

#### 実装方針

**Adafruit BLE API** でカスタム UUID サービスを定義。

---

### 3.9 （将来）電池電圧監視（F7）

**概要：** **提供者** 電池監視（A0＋分圧、回路実装後）。**利用者** アプリ。**関連** F7、`hardware_spec` アナログ節。

#### 境界契約（インタフェース）

**入力**

| ID | 論理名 | 意味 |
|----|--------|------|
| I-BAT-01 | しきい値 | V または％ — TBD |

**出力**

| ID | 論理名 | 意味 |
|----|--------|------|
| D-BAT-01 | フィルタ済み電圧または％ | TBD |

**オペレーション**

| ID | 論理オペレーション | 内容 |
|----|-------------------|------|
| OP-BAT-01 | ADC サンプル | ポーリング間隔は詳細設計 |

**イベント**

| ID | 論理名 | 内容 |
|----|--------|------|
| E-BAT-01 | 低電池検出 | Low Battery 表示・安全停止へ |

#### 前置・制約

未実装間はスタブ／無効でよい（[`features.md`](../1_requirements/features.md) F7）。

#### 実装方針

SAADC または `analogRead` — 詳細設計。

---

### 3.10 開発ホスト／USB（F8）

**概要：** **提供者** USB（ブートローダー／CDC）。**利用者** 開発ホスト。**関連** F8、[Seeed Getting Started](https://wiki.seeedstudio.com/XIAO_BLE/)。

#### 境界契約（インタフェース）

**入力**

| ID | 論理名 | 意味 |
|----|--------|------|
| — | ホストからの書き込み画像 | PlatformIO／ nrfprog 等 |

**出力**

| ID | 論理名 | 意味 |
|----|--------|------|
| D-USB-01 | ログストリーム | 実装定義のテキスト（任意） |

**オペレーション**

| ID | 論理オペレーション | 内容 |
|----|-------------------|------|
| OP-USB-01 | フラッシュ更新 | F8 の開発運用 |
| OP-USB-02 | デバッグシリアル | 任意 |

**イベント**

- 特になし（ホスト駆動）。

#### 前置・制約

[`features.md`](../1_requirements/features.md) F8・非機能「保守性」が正。

#### 実装方針

PlatformIO **upload_port**、Adafruit ブートローダ標準。

---

### 3.11 電源チェーン（ハード主体）

**概要：** **提供者** 昇圧・バリア・保持。**利用者** 全ハード。**関連** F1、[XCL102](https://product.torexsemi.com/en/series/xcl102)。

#### 境界契約（インタフェース）

ソフトウェア API は **なし**。**実効〜2.9V 級**はマージン設計の前提（[`hardware_spec.md`](../1_requirements/hardware_spec.md)）。F7 後は ADC で間接観測。

---

## 4. ブロック間データフロー（論理）

[`block_diagram.md`](block_diagram.md) の「データフロー」と同一思想で、ソフト境界での **定型パケット** を以下とする（詳細設計で構造体名へ）。

| ステップ | 発→着 | 論理データ |
|----------|--------|------------|
| ウェーク | 計時 or IMU → アプリ | ウェーク理由 |
| 測定 | アプリ → SGP40 ドライバ → アプリ | RAW／処理済み Index |
| 推計 | アプリ内 | Index／履歴 → eCO₂・換気・トレンド |
| 表示 | アプリ → e-Paper ドライバ | 描画モデル（数値・インデックス・アイコン） |
| BLE（IMU のみ） | アプリ ↔ BLE ↔ Central | アーキテクチャ表の特性どおり |
| 休眠 | アプリ → スリープ制御 | 次回ウェーク条件 |

---

## 5. BLE GATT と論理データ対応（ドラフト同期）

[`architecture.md`](../1_requirements/architecture.md) の特性と **アプリが保持する論理データ** の対応は以下とする（UUID は同ファイルの仮値）。

| 特性（論理名） | アプリ側ソース | Notify 方針（要件） |
|----------------|----------------|---------------------|
| VOC Index | SGP40＋ガスインデックスパイプライン出力 | 任意 |
| eCO₂ | §3.5 推計出力 | 任意 |
| サンプルインデックス | アプリ単調カウンタ | 推奨（接続直後プッシュ用途） |
| ファーム識別 | ビルド定数 | 任意 |

---

## 6. I/F のトレーサビリティ

| I/F の要約 | 関連 F-ID／備考 |
|-------------|-----------------|
| 約10分ウェーク→測定パイプライン | F2 |
| VOC Index／RAW 取得（Sensirion 公式スタック） | F2、F3 |
| eCO₂・換気・トレンド生成 | F3 |
| e-Paper 更新（GxEPD2） | F4 |
| スリープ移行・IMU 維持 | F5 |
| IMU ウェーク→同一パイプライン→BLE 60秒・Read／Notify・Write なし | F6 |
| 低電池検出・表示・停止（将来） | F7 |
| USB フラッシュ・開発シリアル | F8 |
| UC との詳細対応 | [`../1_requirements/use_case_diagram.md`](../1_requirements/use_case_diagram.md) |

---

## 関連ドキュメント

- [block_diagram.md](block_diagram.md)
- [`../SCOPE.md`](../SCOPE.md)
- [`../1_requirements/architecture.md`](../1_requirements/architecture.md)
- [`../1_requirements/hardware_spec.md`](../1_requirements/hardware_spec.md)
- [`../1_requirements/hardware_documentation/README.md`](../1_requirements/hardware_documentation/README.md)
