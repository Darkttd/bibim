#!/usr/bin/env bash
# Smoke tests for the C# tools on Mono: run two console programs through the CLI host
# (tools/Bibim.Baaaaaab.Cli) and check their results.
#
# Usage: .github/scripts/tools-smoke.sh [Configuration]   (default: Release)
# Build the CLI first, for example:
#   xbuild /p:Configuration=Release tools/Bibim.Baaaaaab.Cli/Bibim.Baaaaaab.Cli.csproj
set -euo pipefail

root=$(cd "$(dirname "$0")/../.." && pwd)
configuration=${1:-Release}
cli="$root/tools/Bibim.Baaaaaab.Cli/bin/x86/$configuration/Bibim.Baaaaaab.Cli.exe"
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT

if [ ! -f "$cli" ]; then
    echo "error: $cli not found; build the CLI first" >&2
    exit 1
fi

# Run from bin/ so that the CLI loads bin/Plugin/*.dll, as the README describes.
cd "$root/bin"

echo "== cppfull.h: generate the all-in-one header of include/Bibim"
header="$work/All.h"
log="$work/cppfull.log"
if ! mono "$cli" console /class:cppfull.h /proj:Bibim /src:../include/Bibim "/dest:$header" < /dev/null > "$log" 2>&1; then
    cat "$log"
    echo "error: cppfull.h failed" >&2
    exit 1
fi
# The header includes every header in include/Bibim, written as "#  include <Bibim/Name.h>".
expected=$(find ../include/Bibim -maxdepth 1 -name '*.h' | wc -l)
actual=$(grep -cE '^#[[:space:]]*include <Bibim/[^>]+\.h>' "$header" || true)
if [ "$actual" -ne "$expected" ]; then
    cat "$log"
    echo "error: $header includes $actual headers, expected $expected" >&2
    exit 1
fi
echo "ok: $actual includes"

echo "== AssetServer: start, then stop with \"exit\" on standard input"
log="$work/AssetServer.log"
status=0
printf 'exit\n' | timeout 60 mono "$cli" console /class:AssetServer > "$log" 2>&1 || status=$?
cat "$log"
if [ "$status" -ne 0 ]; then
    echo "error: AssetServer exited with $status" >&2
    exit 1
fi
if ! grep -q 'AssetServer closed.' "$log"; then
    echo "error: AssetServer did not log that it closed" >&2
    exit 1
fi
echo "ok"
