#!/bin/sh
set -eu

bin=${1:-./qbe}
asm=${IE64ASM:-../IntuitionEngine/sdk/bin/ie64asm}
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM

for level in 0 2 3; do
	"$bin" -t ie64 -O"$level" -mfpext -mfma test/ie64-fpext/pairs.ssa > "$tmp/pairs.s"
	awk '
		/^[a-z_]+:$/ { fn = substr($1, 1, length($1)-1) }
		$1 == ".size" { fn = "" }
		fn == "sincos_pressure" && $1 == "dsincos" {
			pressure = 1
			if ($2 != "f0," && $2 != "f4,") bad = 1
		}
		fn == "sincos_pressure" && $1 == "dstore" && /\(r31\)$/ && !pressure { saved++ }
		fn == "sincos_pressure" && $1 == "dload" && /\(r31\)$/ && pressure { restored++ }
		fn == "cross_call" && $1 == "jsr" && $2 == "clobber" { called = 1 }
		fn == "cross_call" && $1 == "dstore" && called { cos_after_call = 1 }
		fn == "pair_sincos" || fn == "wide_sincos" || fn == "cos_only" || fn == "two_pairs" {
			if ($1 ~ /^(fmov|dmov|fload|dload)$/ || ($1 ~ /^(fstore|dstore)$/ && $2 ~ /^f(8|9|1[0-5]),$/))
				bad = 1
			if ($1 == "fsincos" || $1 == "dsincos") {
				d = substr($2, 2, length($2)-2) + 0
				alignment = $1 == "dsincos" ? 4 : 2
				if (d >= 8 || d % alignment || ((fn == "two_pairs") && seen[d]++))
					bad = 1
				count[fn]++
			}
		}
		(fn == "scalar_fma" || fn == "wide_fma") && $1 !~ /:$/ {
			instructions[fn]++
			if ($1 == "fma" || $1 == "dfma") fused[fn]++
		}
		END {
			exit (bad || count["pair_sincos"] != 1 || count["wide_sincos"] != 1 ||
			    count["cos_only"] != 1 || count["two_pairs"] != 2 ||
			    instructions["scalar_fma"] != 2 || instructions["wide_fma"] != 2 ||
			    !fused["scalar_fma"] || !fused["wide_fma"] ||
			    !pressure || saved < 2 || restored < 2 || !cos_after_call)
		}
	' "$tmp/pairs.s" || {
		printf "%s\n" "-O$level: unwanted FP moves, saves or destination in the pair fixture" >&2
		exit 1
	}
	"$asm" -c -o "$tmp/pairs.o" "$tmp/pairs.s"
done
