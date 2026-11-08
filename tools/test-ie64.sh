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
	# brimm-*.ssa run twice below: with and without -mbranch-imm.
	case "$(basename "$in")" in
	brimm-*) continue ;;
	esac
	gold=${in%.ssa}.s
	printf '%-45s' "$(basename "$in")..."
	# The golden files pin the call sequences of the back end, so they
	# are generated without inlining; inline-*.ssa tests run with it.
	case "$(basename "$in")" in
	inline-*) noinline= ;;
	*) noinline=-fno-inline ;;
	esac
	"$bin" -t ie64 $noinline -o "$tmp" "$in"
	cat "$gold" > "$expected"
	if diff -u "$expected" "$tmp"
	then
		echo "[ok]"
	else
		echo "[failed]"
		exit 1
	fi
done

# Compare-and-branch with an immediate.  The .s golden is the output with
# -mbranch-imm.  The .off.s golden is the output without it and must stay
# the output of the compiler before the option existed.
for in in test/ie64/brimm-*.ssa
do
	for mode in on off
	do
		case $mode in
		on) flag=-mbranch-imm; gold=${in%.ssa}.s ;;
		off) flag=; gold=${in%.ssa}.off.s ;;
		esac
		printf '%-45s' "$(basename "$in") ($mode)..."
		"$bin" -t ie64 -fno-inline $flag -o "$tmp" "$in"
		cat "$gold" > "$expected"
		if diff -u "$expected" "$tmp"
		then
			echo "[ok]"
		else
			echo "[failed]"
			exit 1
		fi
	done
done

printf '%-45s' '-mbranch-imm needs the ie64 target...'
if "$bin" -t amd64_sysv -mbranch-imm -o "$tmp" test/ie64/branch-eq.ssa 2>/dev/null
then
	echo "[failed]"
	exit 1
fi
echo "[ok]"

printf '%-45s' 'legacy IE64 unit-prefix option...'
if "$bin" -t ie64 -fie64-unit-prefix=compat -o "$tmp" test/ie64/branch-eq.ssa
then
	echo "[ok]"
else
	echo "[failed]"
	exit 1
fi

printf '%-45s' 'spilled jnz condition...'
if "$bin" -t ie64 -o "$tmp" test/ie64/jnz-spilled-cond.ssa.compile && grep -F 'load.l r30' "$tmp" >/dev/null
then
	echo "[ok]"
else
	echo "[failed]"
	exit 1
fi

printf '%-45s' 'initialised IE64 common data...'
if "$bin" -t ie64 -o "$tmp" test/ie64/common-initialized.ssa.invalid 2>"$expected"
then
	echo "[failed]"
	exit 1
fi
if grep -F "invalid common data definition" "$expected" >/dev/null
then
	echo "[ok]"
else
	cat "$expected" >&2
	echo "[failed]"
	exit 1
fi

printf '%-45s' 'nonzero untyped floating constant...'
if "$bin" -t ie64 -o "$tmp" test/ie64/float-untyped-nonzero.ssa.invalid 2>"$expected"
then
	echo "[failed]"
	exit 1
fi
if grep -F "IE64 floating copy has an untyped constant" "$expected" >/dev/null
then
	echo "[ok]"
else
	cat "$expected" >&2
	echo "[failed]"
	exit 1
fi

printf '%-45s' 'IE64 linkage on non-IE64 target...'
if "$bin" -t amd64_sysv -o "$tmp" test/ie64/hidden-linkage.ssa 2>"$expected"
then
	echo "[failed]"
	exit 1
fi
if grep -F "linkage is not supported for target amd64_sysv" "$expected" >/dev/null
then
	echo "[ok]"
else
	cat "$expected" >&2
	echo "[failed]"
	exit 1
fi

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
		"$asm" -c -Werror -o "$image" "$gold" >/dev/null
	done
	echo "assembler syntax...                              [ok]"
fi

"$bin" -t ie64 -o "$tmp" test/ie64/branch-eq.ssa
if grep -q '^\.L1:' "$tmp" && grep -q '^\.L2:' "$tmp"
then
	echo "object-local labels...                           [ok]"
else
	echo "object-local labels...                           [failed]"
	exit 1
fi

"$bin" -t ie64 -o "$tmp" test/ie64/wide-spill-align.ssa
if grep -q 'store.q r[0-9]*, 8(r31)' "$tmp" && ! grep -q 'store.q r[0-9]*, 4(r31)' "$tmp"
then
	echo "wide spills after four-byte locals...             [ok]"
else
	echo "wide spills after four-byte locals...             [failed]"
	exit 1
fi
