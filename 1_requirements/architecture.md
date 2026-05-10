# アーキテクチャ概要

## 製品コンセプト

**正本：** [features.md](features.md) **§1 製品コンセプト（概要）〈正本〉**。本ファイルは構成・データフロー視点のみ。

## 構成要素

- **システム境界**: **機器本体**（単三・昇圧・MCU・センサ・e-Paper 等の **部材 BOM・電気的レール の正：** [hardware_spec.md](hardware_spec.md)）／**ユーザー視認（e-Paper）**／**スマホ等 BLE Central**（**広告ウィンドウ 60秒**、**Read メイン／Notify 可**、**Write なし**。要件は **`features`** **F6**・**§3**）。
- **主要コンポーネント**
  - **電源**: 単三省電・昇圧・逆流防止・電解コン保持→実効〜2.9V級。型番一覧と結線は **`hardware_spec`**。
  - **ハブ MCU**: **Seeed XIAO nRF52840 Sense**。間欠サイクル、e-Paper、電力状態、内蔵 **BLE**。**振るでのウェーク**の HW／割込・sleep 両立は **`hardware_spec`**「IMU とスリープ」。
  - **環境入力**: **Sensirion SGP40**。VOC／eCO₂パイプラインは **`features` F3**。
  - **情報出力**: **Waveshare 2.9inch e-Paper**（SPI、296×128）。**GxEPD2**。マニュアル・アンカーは **`hardware_spec`**。
- **データフロー（据え置き・通常）**: **約10分ごとのウェーク**（計時は基本設計で選定。`features.md` 脚注参照）→ **SGP40** → **VOC Index** → **eCO₂** → **インデックス付きで RAM／Flash に保持しうる** → **SPI で e-Paper 更新**（**ユーザーが現状把握**）→ **System OFF／Deep Sleep**。  
  **データフロー（BLE・IMU）**: **MCU 側スリープ中も IMU が振動を検知**（HW／具体的 sleep 構成は `hardware_spec`・実装で確定／追随可）→ ウェーク → **SGP40 → VOC Index → eCO₂ → e-Paper 更新** → **connectable 広告を 60秒** → **Central が GATT Read／（CCCD 有効化時）Notify を受領できる** → **窓終了でラジオ停止・再スリープ**。
- **低電池**: **A0 等による電圧監視が使える状態** で **しきい（V／％は基本設計で確定）** に応じ **Low Battery** 表示と **安全停止**。**A0 未実装間** は `features.md` **F7** のとおり **USB を主とする開発運用** とし電池しきいは未適用または暫定。

## 技術スタック（要件固定）

| 区分 | 採用 |
|------|------|
| IDE／ビルド | **PlatformIO IDE** |
| フレームワーク | **Arduino Framework**／**Adafruit nRF52 Board Support** |
| e-Paper | **GxEPD2**（Waveshare 公式より軽量・高速を意図） |
| VOC センサ | **Sensirion Arduino I²C SGP40** 系ライブラリ |
| BLE（最小スコープ） | **Adafruit BLE API／nRF SoftDevice 前提の Arduino 実装**。**カスタム UUID**。**Write・Indicate なし**。**Read と Notify を組み合わせ可能**（下表ドラフト）。OTA・ペアリング高度化は本条スコープ外。 |
| 省電 | **nRF52840／nRF52 固有パワーマネジメント命令** と Arduino 側 sleep API。**ラジオは F6 の 60秒／回 のみ**。IMU ウェークとの両立可否は実装フェーズで検証（**`hardware_spec`**） |

**ホスト OS／エディタ／USB 転送の唯一の正：** [features.md](features.md) **F8** および **非機能（保守性）**。本条はプラットフォーム・スタック表のみ維持する。

## BLE GATT（要件：ドラフト／基本設計で UUID と型を確定）

**カスタムサービス UUID（仮）**: `11111101-2222-4333-8444-A55555550001`。実装開始前または基本設計で正式 UUID に差し替える。

**特性（ドラフト）。Write／Indicate なし。** 列挙順はレビュー用。

| # | 論理名 | 意味 | 値の型（暫定） | Notify | 特性 UUID（仮・128-bit） |
|---|--------|------|----------------|:------:|--------------------------|
| 1 | VOC Index | 当ウェークの VOC Index | uint16 LE | ○ 任意 | `11111101-2222-4333-8444-A55555551101` |
| 2 | eCO₂ | 当ウェークの推計値 | uint16 LE | ○ 任意 | `11111101-2222-4333-8444-A55555551102` |
| 3 | サンプルインデックス | **単調増加の系列番号**（約10分周期サンプルと 1:1 想定。壁時計は装置に不要） | uint32 LE | **○ 推奨**（接続直後のプッシュ等） | `11111101-2222-4333-8444-A55555551103` |
| 4 | ファームウェア識別 | ビルド識別子 | blob または固定長 | ○ 任意 | `11111101-2222-4333-8444-A55555551104` |

- **Notify**: **`CCCD` で購読**。**ポートフォリオ最小**ならインデックス等 **1〜2種**のみ Notify で十分。履歴の連続バースト Notify は **`features`** のとおり必須化しない。長いデータは **Read** と両立させる開発判断あり。  
- **時刻変換**：**スマホ側**。装置はインデックス等のみ。  
- Central は **F6 の 60秒ウィンドウ**内で接続する想定。

## ピンアサイン

**アサインテーブルの唯一の正：** [hardware_spec.md](hardware_spec.md) **「GPIO／ピンアサイン」**。

## 制約・前提

- **ウェーク／スリープ／BLE ウィンドウのユーザーストーリーと適応変更**：**正：** [features.md](features.md) **§3「休眠・ウェーク構成の適応変更」**。HW を伴う復帰条件は **`hardware_spec`**「IMU とスリープ」。
- **実効〜2.9V級・各部材許容電圧・電源チェーン**：**正：** [hardware_spec.md](hardware_spec.md)（電気的特性・電源チェーン）。
- **表示更新**: **電子インク** のため書き換えは低頻度＋イベント駆動（IMU）。
- **真の CO₂**: 非測定。**VOC ベースの eCO₂ は独自アルゴリズム**。表示上の脚注・ユーザー説明は別途検討可。

## 関連

- ピン・電源詳細: [hardware_spec.md](hardware_spec.md)
- 機能 ID: [features.md](features.md)
- 詳細コンポーネント接続: [../2_basic_design/block_diagram.md](../2_basic_design/block_diagram.md)
