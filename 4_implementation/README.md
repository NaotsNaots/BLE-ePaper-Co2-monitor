# BLE e-Paper 環境モニター — Implementation

**PlatformIO** 単一プロジェクト（`4_implementation/` 直下）。  
要件のスタック: [../1_requirements/architecture.md](../1_requirements/architecture.md)。

## Layout

```
4_implementation/
├── .venv/             # （ローカル）PlatformIO CLI 用 Python venv／`pip install platformio`。`.pio/` と名前が被らない
├── platformio.ini
├── src/
│   └── main.cpp       # エントリ（Arduino setup/loop）
├── include/           # 自前ヘッダ（任意）
├── lib/               # ローカルライブラリ（任意）
├── test/
│   └── hw/            # ハード単体スケッチ（`pio run -e test_*_only`、`src_dir` で切替）
└── README.md
```

## Cursor + PlatformIO IDE 拡張（推奨）

要件 F8（Ubuntu／Cursor）と揃えやすい開発フローです。CLI の `pio` を直接打たなくても、拡張からビルド・書き込み・シリアルを実行できます（裏側は PlatformIO Core）。

1. **拡張機能**を開き、`PlatformIO IDE`（発行元: PlatformIO）をインストールする。初回に PlatformIO Core の取得・セットアップが走ることがある。
2. Cursor で **`4_implementation` フォルダを開く**（`platformio.ini` がある階層をワークスペースのルートにする。リポジトリ全体を開いた場合は、「追加のフォルダー」で `4_implementation` を入れるか、このパスだけ開く）。
3. 左の **PlatformIO** アイコン（アリのロゴ）から **Build**／**Upload**／**Serial Monitor** を使う。初回ビルドでは Seeed の `platform-seeedboards` 取得に時間がかかる。
4. ターミナルで **`pio` が PATH に無いとき** は、このフォルダ内の `./.venv/bin/pio` を使う（`.pio/` は **ビルド成果物キャッシュ用**であり、venv ではない）。
5. （参考）環境により **`.pio-venv`** という名前が残ることがある。**`.venv`** に統一したいときは、この README の「CLI セットアップ」を参照する。

**Linux（USB 書き込み・シリアル）:** デバイスにアクセスできないときは、[PlatformIO の udev 手順](https://docs.platformio.org/en/latest/core/installation/udev-rules.html)に沿ってユーザー権限を確認する。

---

## CLI セットアップ（初回のみ）

ターミナルから `pio` を使う場合、このプロジェクトでは **`.venv/` にインストールされた PlatformIO** を使う。以下を一度だけ実行する。

```bash
cd 4_implementation
python3 -m venv .venv                        # プロジェクト専用の Python 環境を作成
.venv/bin/pip install 'platformio>=6.1,<7'   # その中に PlatformIO をインストール
rm -rf .pio-venv   # IDE 拡張が自動生成した別 venv が残っていれば削除
```

> **`.venv/` と `.pio/` の違い:**
> `.venv/` は PlatformIO CLI 本体を入れる Python 仮想環境。
> `.pio/` はビルド成果物のキャッシュ置き場であり、venv ではない。

---

## `pio` コマンドの呼び出し方

`pio` は `.venv/bin/` の中にあるため、グローバルには見つからない。呼び出し方は2通りある。

**方法A — 直接パス指定（単発実行向け）**

```bash
.venv/bin/pio run
```

PATH を変えないため、他のターミナルや環境に影響しない。

**方法B — venv を activate する（作業セッション向け）**

```bash
source .venv/bin/activate   # .venv/bin/ が PATH の先頭に追加される
pio run                     # 以降 pio だけで打てる
deactivate                  # セッション終了時に元に戻す
```

activate は `.venv/bin/` を `PATH` の先頭に追加するだけであり、動作結果は方法Aと同じ。
**このドキュメント内のコマンド例は方法A（直接パス指定）で統一している。**

---

## ビルドと書き込み

### ビルド

```bash
.venv/bin/pio run
```

初回は Seeed の `platform-seeedboards` の取得に時間がかかる。

### 書き込み

USB 接続後、デバイス名を確認してから書き込む。

```bash
# デバイス名の確認
ls -l /dev/ttyACM* /dev/serial/by-id/*Seeed* /dev/serial/by-id/*2886* 2>/dev/null
.venv/bin/pio device list

# 書き込み（ポートを明示するのが確実）
.venv/bin/pio run -t upload --upload-port /dev/ttyACM0
```

`/dev/ttyACM` が出ず `/dev/ttyS*` だけになるときは USB データ線・抜き差し・`dmesg` を確認する。

### シリアルモニタ

```bash
.venv/bin/pio device monitor --port /dev/ttyACM0
```

---

## テスト

テスト全体の説明は [`test/README.md`](test/README.md) を参照。

### ハード単体スケッチ（配線確認）

```bash
# SGP40（SRAW_VOC を 2秒ごとに出力）
.venv/bin/pio run -e test_sgp40_only -t upload
.venv/bin/pio device monitor -e test_sgp40_only

# e-Paper（白黒交互に 8秒ごとに反転）
.venv/bin/pio run -e test_epaper_only -t upload
.venv/bin/pio device monitor -e test_epaper_only
```

### Native unit test（PC のみ、ハード不要）

```bash
.venv/bin/pio test -e native_eco2mapper
.venv/bin/pio test -e native_statemachine
```

---

## 設計準拠

実装レビューは `../3_detailed_design/` と [../reviews/code_review_template.md](../reviews/code_review_template.md) で照合する。
