#!/usr/bin/env bash
# Replace REPLACE_ME_PROJECT_NAME with your display title across text files under this tree.
set -euo pipefail

if [[ "${1:-}" == "" ]]; then
  printf 'Usage: %s "Your Project Title"\n' "$0" >&2
  exit 1
fi

export INIT_PROJECT_DISPLAY_NAME="$1"

python3 - <<'PY'
import os
from pathlib import Path

name = os.environ["INIT_PROJECT_DISPLAY_NAME"]
token = "REPLACE_ME_PROJECT_NAME"
root = Path.cwd()
skip_dirs = {".git", ".venv", "venv", "__pycache__", ".pytest_cache"}
suffixes = {".md", ".py", ".txt", ".yaml", ".yml", ".ini", ".toml"}

changed = []
for path in root.rglob("*"):
    if not path.is_file():
        continue
    if path.suffix.lower() not in suffixes:
        continue
    rel = path.relative_to(root)
    if any(p in skip_dirs for p in rel.parts):
        continue
    try:
        text = path.read_text(encoding="utf-8")
    except (UnicodeDecodeError, OSError):
        continue
    if token not in text:
        continue
    path.write_text(text.replace(token, name), encoding="utf-8")
    changed.append(path)

if not changed:
    print(f'No occurrences of "{token}" found under {root}.')
else:
    for p in changed:
        print(p)
    print(f'Updated display name → {name}')
PY
