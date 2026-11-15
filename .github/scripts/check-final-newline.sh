#!/usr/bin/env bash
# Lists tracked text files that do not end with a newline and fails if there are any.
# Third-party code (extlibs/, tools/References/) and prebuilt tools (bin/) are skipped.
# Empty files are fine.
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

missing=0
while IFS= read -r -d '' file; do
  if [ -n "$(tail -c 1 -- "$file")" ]; then
    echo "$file"
    missing=$((missing + 1))
  fi
done < <(git grep -I -z -l '' -- . ':!extlibs/' ':!tools/References/' ':!bin/')

if [ "$missing" -ne 0 ]; then
  echo "$missing file(s) above do not end with a newline. Add one, keeping the file's line endings (CRLF or LF)." >&2
  exit 1
fi
