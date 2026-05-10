# Git / GitHub 運用ガイド（モノレポ）

**正本**: このリポジトリルートにある単一 `.git`（設計ドキュメント + `4_implementation/`）。  
マイコン上にソースツリーを置く想定はしません。クローン・コミット・タグ付けは**開発 PC（および CI）**で行い、ビルド成果物はプログラマで書き込みます。

いつも **リポジトリ全体を `git clone` する**運用でよいです。ビルド・書き込み手順は [`4_implementation/README.md`](4_implementation/README.md) を参照してください。

すべてのパスの例では `<YOUR_CLONE_PATH>` とリポジトリ URL のプレースホルダーを置き換えてください。

---

## 1. PC：リポジトリの取得・更新

```bash
git clone https://github.com/<YOUR_USERNAME>/<YOUR_REPO>.git
cd <YOUR_REPO>
```

```bash
git pull origin main
```

設計書とファームウェア実装の編集・コミットはここで行い、`git push origin main` でリモートに反映します。

---

## 2. GitHub へ初回公開（PC）

空のリポジトリを GitHub で作成したうえで:

```bash
cd <YOUR_CLONE_PATH>
git remote add origin https://github.com/<YOUR_USERNAME>/<YOUR_REPO>.git
git push -u origin main
```

GitHub CLI 例: `gh repo create <YOUR_REPO> --public --source=. --remote=origin --push`

---

## 3. ブランチとリリース（当面）

**一人開発**のため、基本は **`main` へ直接コミット・`git push origin main`** で進める。フィーチャーブランチや PR 必須の運用はとらない。

開発が終盤で煮詰まったタイミングで、ブランチ／レビュー／リリース手順を変える可能性がある。その場合は **本ファイルを更新するか、都度の指示に従う**（タグや GitHub Releases をどうするかも、そのときに決めてよい）。

---

## 4. 管理レビュー（PC）

詳細設計と実装を並べて照合し、結果・メモを `reviews/` に残す。

**推奨ウィンドウ配置（エディタ／ブラウザ）:**

```
左：3_detailed_design/  中央：4_implementation/  右：reviews/
```

コードレビュー観点のチェックリストは [`reviews/code_review_template.md`](reviews/code_review_template.md)。

---

## 5. バイナリとシークレット

- **ビルド成果物**（`.elf` / `.bin` / `.hex` / `.uf2` など）は **Git にコミットしない**。生成物は **`pio run` 等でソースから再現**する。既定の出力は `4_implementation/.pio/` 配下にあり、`4_implementation/.gitignore` で除外済み。手元でコピーを置く場合は **リポジトリに入れない**よう、ルート `.gitignore` で `4_implementation/` 直下の上記拡張子および `export/`・`dist/` を無視する。
- **配布やスナップショット**が必要なときは **Git の対象外**とし、[GitHub Releases](https://docs.github.com/en/repositories/releasing-projects-on-github) に添付する、CI の Artifacts に残す、など **別経路**で渡す（タグ＝ソースのリビジョン、添付＝そのときのイメージ、の切り分けがしやすい）。
- **認証情報・キー**（BLE LTK、API キー、署名鍵など）はコミットしない。必要ならテンプレと実機用ローカル設定の切り分けを `4_implementation/README.md` 等に明記する。
