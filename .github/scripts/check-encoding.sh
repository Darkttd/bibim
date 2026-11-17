#!/usr/bin/env bash
# Lists tracked text files that are not UTF-8 or that start with a byte order mark, and fails if
# there are any. Third-party code (extlibs/, tools/References/) and prebuilt tools (bin/) are skipped.
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

checked=0
with_bom=0
not_utf8=0
while IFS= read -r -d '' file; do
  checked=$((checked + 1))
  if [ "$(head -c 3 -- "$file" | od -An -tx1 | tr -d ' \n')" = "efbbbf" ]; then
    echo "$file: starts with a UTF-8 byte order mark"
    with_bom=$((with_bom + 1))
  fi
  if ! iconv -f UTF-8 -t UTF-8 -- "$file" >/dev/null 2>&1; then
    echo "$file: is not valid UTF-8"
    not_utf8=$((not_utf8 + 1))
  fi
done < <(git grep -I -z -l '' -- . ':!extlibs/' ':!tools/References/' ':!bin/')

# A failing git grep inside the process substitution would otherwise look like a pass.
if [ "$checked" -eq 0 ]; then
  echo "No files were checked; git grep found nothing or failed." >&2
  exit 1
fi

# git grep -I treats UTF-16 text as binary (it contains NUL bytes), so look for a UTF-16 BOM
# among all tracked files as well.
while IFS= read -r -d '' file; do
  case "$(head -c 2 -- "$file" | od -An -tx1 | tr -d ' \n')" in
    fffe | feff)
      echo "$file: is UTF-16"
      not_utf8=$((not_utf8 + 1))
      ;;
  esac
done < <(git ls-files -z -- . ':!extlibs/' ':!tools/References/' ':!bin/')

if [ "$with_bom" -ne 0 ] || [ "$not_utf8" -ne 0 ]; then
  echo "Text files must be UTF-8 without a byte order mark." >&2
  if [ "$with_bom" -ne 0 ]; then
    echo "Remove a BOM with: sed -i '1s/^\xEF\xBB\xBF//' <file>" \
         "(Visual Studio adds one when it saves .sln and project files)." >&2
  fi
  if [ "$not_utf8" -ne 0 ]; then
    echo "Convert other encodings with, for example: iconv -f CP949 -t UTF-8 <file>" >&2
  fi
  exit 1
fi
