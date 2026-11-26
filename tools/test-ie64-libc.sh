#!/bin/sh
# SPDX-License-Identifier: MIT
set -eu

sdk=${IE64_LIBC_SYSROOT:?set IE64_LIBC_SYSROOT to the Picolibc SDK}
engine=${IE64_ENGINE:?set IE64_ENGINE to the headless engine}
fixture=$(cd test/ie64-libc && pwd)
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
export IE_NO_IPC=1
export TMPDIR=$tmp XDG_RUNTIME_DIR=$tmp/run
mkdir -p "$XDG_RUNTIME_DIR"
cp "$fixture/exit.ies" "$tmp/exit.ies"

"$sdk/bin/ie64-cproc" --sysroot "$sdk" -O2 -ffreestanding -fno-builtin \
    -o "$tmp/libc.ie64" "$fixture/runtime.c"
if ! "$engine" -ie64 -file-root "$tmp" -script-owned-term \
    -script "$tmp/exit.ies" "$tmp/libc.ie64" </dev/null >"$tmp/engine.log" 2>&1; then
	cat "$tmp/engine.log" >&2
	exit 1
fi
printf '%s\n' 'Picolibc memory, MMU seams and square roots passed.'
