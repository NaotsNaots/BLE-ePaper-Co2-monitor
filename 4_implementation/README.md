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
5. （参考）環境により **`.pio-venv`** という名前が残ることがある。**`.venv`** に統一したいときは、この README の「ローカルの PlatformIO を `.venv` にそろえる」を参照する。

**Linux（USB 書き込み・シリアル）:** デバイスにアクセスできないときは、[PlatformIO の udev 手順](https://docs.platformio.org/en/latest/core/installation/udev-rules.html)に沿ってユーザー権限を確認する。

---

## ローカルの PlatformIO を `.venv` にそろえる

拡張が **`.pio-venv`** のような名前を付けても、このリポジトリでは **標準どおり `./.venv/` に PlatformIO Core を置く**運用とする（**`.pio/`** はキャッシュのみで venv と混ぜない）。

```bash
cd 4_implementation
python3 -m venv .venv
.venv/bin/pip install 'platformio>=6.1,<7'
rm -rf .pio-venv   # 同名の環境だけが残っていれば削除してよい
```

ターミナルからは次のように呼ぶ:

```bash
./.venv/bin/pio run
```

---

## Setup / build

[PlatformIO](https://platformio.org/)（上記拡張**または** CLI／上記 `./.venv`）を使い、このディレクトリで作業する。

```bash
cd 4_implementation
pio run                       # PATH にある場合
# または ./.venv/bin/pio run
```

初回は Seeed の `platform-seeedboards` の取得に時間がかかります。

書き込み（USB 接続・ボードのブートローダ状態に合わせる）:

```bash
pio run -t upload
# または ./.venv/bin/pio run -t upload
```

**ポートを明示する（推奨）:** XIAO が **`/dev/ttyACM0` ではない**ことや、**書き込み実行時に USB が無い**と `No such file ... ttyACM0` になります。接続した直後に次で確認する。

```bash
ls -l /dev/ttyACM* /dev/serial/by-id/*Seeed* /dev/serial/by-id/*2886* 2>/dev/null
./.venv/bin/pio device list
```

例（実際のデバイス名に合わせる）:

```bash
./.venv/bin/pio run -t upload --upload-port /dev/ttyACM0
# または /dev/ttyACM1 / by-id 下のシンボリックリンク
```

自動検出が **`/dev/ttyS*`** ばかりになり **`ttyACM` が出ない**ときは、USB（データ線）・直挿し・抜き差し後の `dmesg` を確認する。

シリアルモニタ:

```bash
pio device monitor
# または ./.venv/bin/pio device monitor
# ポート指定: ./.venv/bin/pio device monitor --port /dev/ttyACM0
```

---

## ハード単体スケッチ（配線確認）

`test/hw/` のスタンドアロンスケッチを、`platformio.ini` の **`test_sgp40_only` / `test_epaper_only`** 環境で書き込む。

```bash
# SGP40 のみ（SRAW_VOC を 2秒ごとに出力）
./.venv/bin/pio run -e test_sgp40_only -t upload
./.venv/bin/pio device monitor -e test_sgp40_only

# e-Paper のみ（白黒交互に 8秒ごとに反転）
./.venv/bin/pio run -e test_epaper_only -t upload
./.venv/bin/pio device monitor -e test_epaper_only
```

---

## 設計準拠

実装レビューは `../3_detailed_design/` と [../reviews/code_review_template.md](../reviews/code_review_template.md) で照合する。
