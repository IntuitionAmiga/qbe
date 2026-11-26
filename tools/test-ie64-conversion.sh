#!/bin/sh
set -eu

qbe=$1
sdk=$2
engine=$3
capture=$4
export IE_NO_IPC=1
work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT HUP INT TERM
src=test/ie64-conversion
"${CC:-cc}" -O2 -DHOST_REFERENCE "$src/reference.c" -o "$work/reference"
"$work/reference" > "$work/reference.out"
for level in 0 2; do
	"$qbe" -t ie64 "-O$level" -o "$work/casts.s" "$src/casts.ssa"
	"$sdk/bin/ie64-cproc" "-O$level" -o "$work/casts.ie64" "$src/reference.c" "$work/casts.s"
	cp --reflink=always "$capture" "$work/capture.ies"
	mkdir -p "$work/runtime"
	IE64_BRANCH_PROFILE="$work/ie.pc" XDG_RUNTIME_DIR="$work/runtime" "$engine" -ie64 -file-root "$work" -script-owned-term \
		-script "$work/capture.ies" "$work/casts.ie64" < /dev/null > "$work/engine.log" 2>&1 || {
		cat "$work/engine.log" >&2
		exit 1
	}
	diff -u "$work/reference.out" "$work/ie.out"
done
