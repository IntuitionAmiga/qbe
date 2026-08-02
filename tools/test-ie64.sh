#!/bin/sh

set -eu

bin=${1:-./qbe}
tmp=${TMPDIR:-/tmp}/qbe-ie64-$$.s
expected=${TMPDIR:-/tmp}/qbe-ie64-$$.expected
image=${TMPDIR:-/tmp}/qbe-ie64-$$.ie64
asm=${IE64ASM:-../IntuitionEngine/sdk/bin/ie64asm}
trap 'rm -f "$tmp" "$expected" "$image"' EXIT HUP INT TERM

for in in test/ie64/*.ssa
do
	gold=${in%.ssa}.s
	printf '%-45s' "$(basename "$in")..."
	"$bin" -t ie64 -o "$tmp" "$in"
	cat "$gold" > "$expected"
	if diff -u "$expected" "$tmp"
	then
		echo "[ok]"
	else
		echo "[failed]"
		exit 1
	fi
done

for in in test/ie64-invalid-*.ssa.invalid
do
	printf '%-45s' "$(basename "$in")..."
	if "$bin" -t ie64 -o "$tmp" "$in" 2>"$expected"
	then
		echo "[failed]"
		exit 1
	fi
	if grep -E 'IE64 immediate operand (must be an integer constant|is outside the supported range)' \
		"$expected" >/dev/null
	then
		echo "[ok]"
	else
		cat "$expected" >&2
		echo "[failed]"
		exit 1
	fi
done

printf '%-45s' 'private operations on non-IE64 target...'
if "$bin" -t amd64_sysv -o "$tmp" test/ie64/builtins-all.ssa 2>"$expected"
then
	echo "[failed]"
	exit 1
fi
if grep -F "private IE64 operation" "$expected" >/dev/null
then
	echo "[ok]"
else
	cat "$expected" >&2
	echo "[failed]"
	exit 1
fi

if [ -x "$asm" ]
then
	for gold in test/ie64/*.s
	do
		"$asm" -Werror -o "$image" "$gold" >/dev/null
	done
	echo "assembler syntax...                              [ok]"
fi

"$bin" -t ie64 -fie64-unit-prefix=unit0 -o "$tmp" test/ie64/branch-eq.ssa
if grep -q '^\.Lunit0\.1:' "$tmp" && grep -q '^\.Lunit0\.2:' "$tmp"
then
	echo "unit prefix...                                  [ok]"
else
	echo "unit prefix...                                  [failed]"
	exit 1
fi
