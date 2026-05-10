# Code Review — [Date: YYYY-MM-DD]

## Context

| 項目 | 内容 |
|------|------|
| **レビュアー／参加者** | |
| **レビュー対象コミット／ブランチ** | |
| **対象デバイス／環境** （該当する場合） | |

## Files reviewed

- [ ] `4_implementation/src/` — ソースコード
- [ ] `4_implementation/test/` — PlatformIO テスト（使用時）
- [ ] `4_implementation/platformio.ini` — ビルド・依存・ターゲット設定

## Design alignment（設計準拠）

以下は **このプロジェクトの設計ファイル名／パスをプロジェクト実態にあわせて書き換え** してください。

### vs. `2_basic_design/block_diagram.md`

- [ ] ブロック構成・コンポーネント境界が図・説明と一致している
- [ ] （ハードがある場合）入出力ポート・バスの利用が記載どおり／理由付き逸脱
   
### vs. `3_detailed_design/class_diagram.md`（またはモジュール構成見出し）

- [ ] 主要モジュール／クラスの責務が設計と一致している（名称はマッピング表で明示してよい）
- [ ] 依存関係が設計の向き／抽象化と矛盾していない
   
### vs. `3_detailed_design/state_machine.md`

- [ ] メイン状態と遷移条件がコードの分岐／ループで表現できている
   
### vs. `3_detailed_design/sequence_diagram.md`

- [ ] 代表シナリオの処理順・外部 I/F 呼び出しが整合している
   
## Functional checks（機能）

（プロジェクトの `1_requirements/features.md` と `features` 一覧から転記してチェックする）

| ID | 観点 | OK |
|----|------|---|
|  | （例）起動および初期化 | [ ] |
|  | （例）異常入力時の扱い | [ ] |

## Non-functional checks（非機能）

- [ ] （例）ログ／観測性：ログレベル、失敗パスのログ
- [ ] （例）堅牢性：例外・タイムアウト・再接続などの規定と実装
- [ ] （例）性能／電力：`features.md` で定義した制約との整合
   
## Issues found

### Critical

| Issue | Location | Status | Notes |
|-------|----------|--------|-------|
|       |          | OPEN   | |

### Minor / suggestions

- [ ]

## Approvals

| Role           | Status   | Signature | Date |
|----------------|----------|-----------|------|
| Code review    | [ ] OK   |           | |
| Design review  | [ ] OK   |           | |

## Action items

- [ ]

## Notes

<!-- 追加コメント -->
