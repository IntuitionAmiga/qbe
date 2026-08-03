#!/bin/sh

set -eu

bin=${1:-./qbe}
fixture=${2:-test/opt-levels.ssa.fixture}
tmp=${TMPDIR:-/tmp}/qbe-opt-$$
trap 'rm -f "$tmp".*' EXIT HUP INT TERM

for level in 0 1 2 3
do
	"$bin" -t ie64 -O"$level" -o "$tmp.$level.s" "$fixture"
	"$bin" -t ie64 -O"$level" -dMG "$fixture" 2>"$tmp.$level.log"
	"$bin" -t amd64_sysv -O"$level" -dK "$fixture" 2>"$tmp.$level.ifconvert.log"
done

if cmp -s "$tmp.0.s" "$tmp.1.s"
then
	echo "-O0 and -O1 generated identical output" >&2
	exit 1
fi
grep -F "load.l" "$tmp.0.s" >/dev/null
! grep -F "load.l" "$tmp.1.s" >/dev/null
! grep -F "> After load elimination:" "$tmp.0.log" >/dev/null
grep -F "> After load elimination:" "$tmp.1.log" >/dev/null
! grep -F "> After if-conversion:" "$tmp.1.ifconvert.log" >/dev/null
grep -F "> After if-conversion:" "$tmp.2.ifconvert.log" >/dev/null
grep -F "> After if-conversion:" "$tmp.3.ifconvert.log" >/dev/null
cmp "$tmp.2.s" "$tmp.3.s"

if "$bin" -t ie64 -O4 -o "$tmp.invalid.s" "$fixture" 2>"$tmp.invalid.err"
then
	echo "invalid optimisation level was accepted" >&2
	exit 1
fi
grep -F "invalid optimization level" "$tmp.invalid.err" >/dev/null

echo "QBE optimisation levels...                         [ok]"
