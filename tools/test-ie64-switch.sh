#!/bin/sh

set -eu

bin=${1:-./qbe}
asm=${IE64ASM:-../IntuitionEngine/sdk/bin/ie64asm}
ld=${IE64LD:-$(dirname "$asm")/ie64ld}
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM

cat > "$tmp/start.s" <<'ASM'
.section .text,"ax"
.global _start
_start:
	jsr pick
	jsr call_switch
	halt
ASM
"$asm" -c -Werror -o "$tmp/start.o" "$tmp/start.s" >/dev/null
for opt in -O0 -O2
do
	for branch in "" -mbranch-imm
	do
		"$bin" -t ie64 "$opt" $branch -o "$tmp/switch.s" test/ie64/switch/phi.ssa
		rg -q '^[[:space:]]+jmp \(r30\)$' "$tmp/switch.s"
		test "$(rg -c '^[[:space:]]+dc\.q \.L[0-9]+$' "$tmp/switch.s")" -eq 6
		if [ "$opt" = -O2 ] && rg -q '^[[:space:]]+jsr offset$' "$tmp/switch.s"; then
			printf '%s\n' 'offset was not inlined before the switch' >&2
			exit 1
		fi
		"$asm" -c -Werror -o "$tmp/switch.o" "$tmp/switch.s" >/dev/null
		"$ld" -base 0x100001000 -stack 0x10009f000 -o "$tmp/high.ie64" \
			"$tmp/start.o" "$tmp/switch.o"
	done
done

if "$bin" -t amd64_sysv test/ie64/switch/phi.ssa > /dev/null 2>"$tmp/error"; then
	exit 1
fi
rg -q 'switch needs the ie64 target' "$tmp/error"
printf "%s\n" "IE64 switch tests passed"
