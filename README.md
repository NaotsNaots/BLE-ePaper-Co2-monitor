# BLE e-Paper 環境モニター（VOC／eCO₂ 据え置き）

単三省電・**Seeed XIAO nRF52840 Sense**・**Sensirion SGP40**・**Waveshare 2.9inch e-Paper** を前提とした据え置き環境情報表示と、IMU ウェーク後の **BLE（60秒ウィンドウ・Read メイン／Notify 可・Write なし）** を扱う設計・実装リポジトリです。

ドキュメントはウォーターフォール型のフェーズフォルダ（要件 → 基本設計 → 詳細設計 → 実装）と成果物台帳（[`SCOPE.md`](SCOPE.md)）、レビュー記録で整理されています。**単体運用**ではこのディレクトリを別リポジトリのルートとしてそのまま使えます。**親リポジトリ直下に置いている場合**は、親側のソースは変えず本サブツリーのみ運用するとよいです。

初期化するときは、テンプレートのルート（`init-from-template.sh` があるディレクトリ）に移ってから実行し、表示名を置換してください。

```bash
chmod +x init-from-template.sh
./init-from-template.sh "マイ組み込みプロジェクト"
```

その後、テンプレートを置いたフォルダを Git のルートとして初期化するか別リモートへ pushし、GitHub の **Template repository** にしたいときは Repository settings で有効にできます。

---

## Project Structure

```
.
├── 1_requirements/
│   ├── README.md
│   ├── architecture.md
│   ├── hardware_spec.md
│   ├── hardware_documentation/
│   │   └── README.md
│   ├── features.md
│   └── use_case_diagram.md
├── 2_basic_design/
│   ├── README.md
│   └── block_diagram.md
├── 3_detailed_design/
│   ├── README.md
│   ├── class_diagram.md
│   ├── state_machine.md
│   └── sequence_diagram.md
├── 4_implementation/
│   ├── platformio.ini
│   ├── src/
│   ├── include/
│   ├── lib/
│   ├── test/
│   └── README.md
├── reviews/
├── SCOPE.md
├── GIT_WORKFLOW.md
├── init-from-template.sh
└── README.md
```

---

## Git / ワークフツリー構成（任意）

Git／GitHub の運用は [GIT_WORKFLOW.md](GIT_WORKFLOW.md) を参照。マイコン上にリポジトリは置かず、ビルド・書き込みは PC から行います。**設計だけのリポジトリ**なら同ファイルは読み飛ばして構いません。

---

## 管理レビュー

ウィンドウ配置・記録のしかたの**正本**は [GIT_WORKFLOW.md](GIT_WORKFLOW.md) §4。チェックリストは [reviews/code_review_template.md](reviews/code_review_template.md)。

---

## 開発フロー（例）

```
PC（設計・ビルド・リポジトリ）              マイコン（XIAO nRF52840 等）
─────────────────────────────────────────────────────────

1_requirements/
2_basic_design/
3_detailed_design/           設計参照

4_implementation/            pio run / upload ──→ ファームウェア書き込み（Git なし）

reviews/                     レビュー記録
```

詳細な成果物定義・承認基準は [SCOPE.md](SCOPE.md)。

---

```mermaid
flowchart TD
  req ["1_requirements"]
  basic ["2_basic_design"]
  detail ["3_detailed_design"]
  impl ["4_implementation"]
  rev ["reviews"]
  req --> basic --> detail --> impl
  detail -.->|"design alignment"| rev
  impl -.->|"notes"| rev
```

---

## 実装（ファームウェア）

[`4_implementation/README.md`](4_implementation/README.md)（PlatformIO／XIAO nRF52840）を参照してください。
