#!/usr/bin/env bash
# Lists lines in the engine sources (src/ and include/) that contain a tab character and fails if
# there are any. Indent with spaces instead, including after '#' in preprocessor directives.
# The Lua and LuaTinker headers copied into include/ are third-party and kept as they are.
set -euo pipefail

cd "$(git rev-parse --show-toplevel)"

pathspecs=(
  'src/*.h' 'src/*.hpp' 'src/*.inl' 'src/*.c' 'src/*.cpp' 'src/*.mm'
  'include/*.h' 'include/*.hpp' 'include/*.inl' 'include/*.c' 'include/*.cpp' 'include/*.mm'
  ':!include/lua.h' ':!include/luaconf.h' ':!include/lauxlib.h' ':!include/lualib.h'
  ':!include/lua_tinker.h'
)

checked=$(git ls-files -- "${pathspecs[@]}" | wc -l)
# A failing git ls-files would otherwise look like a pass.
if [ "$checked" -eq 0 ]; then
  echo "No files were checked; git ls-files found nothing or failed." >&2
  exit 1
fi

# git grep exits with 1 when nothing matches, which is the passing case here.
status=0
matches=$(git grep -n -I -F -e $'\t' -- "${pathspecs[@]}") || status=$?
if [ "$status" -gt 1 ]; then
  echo "git grep failed with exit code $status." >&2
  exit 1
fi

if [ -n "$matches" ]; then
  # Print only file:line; the line content would show the tabs as invisible whitespace anyway.
  printf '%s\n' "$matches" | cut -d: -f1,2
  count=$(printf '%s\n' "$matches" | wc -l)
  echo "$count line(s) above contain a tab. Use spaces (4 per indent level), keeping the file's line endings (CRLF or LF)." >&2
  exit 1
fi
