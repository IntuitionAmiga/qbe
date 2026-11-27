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
		fn == "sincos_store_bits" && $1 !~ /:$/ {
			store_insns++
			if ($1 == "fsincos") store_pair++
			if ($1 == "fstore" && $3 == "(r1)") store_sin++
			if ($1 == "fstore" && $3 == "4(r1)") store_cos++
		}
		END {
			exit (bad || count["pair_sincos"] != 1 || count["wide_sincos"] != 1 ||
			    count["cos_only"] != 1 || count["two_pairs"] != 2 ||
			    instructions["scalar_fma"] != 2 || instructions["wide_fma"] != 2 ||
			    !fused["scalar_fma"] || !fused["wide_fma"] ||
			    !pressure || saved < 2 || restored < 2 || !cos_after_call ||
			    store_insns != 4 || !store_pair || !store_sin || !store_cos)
		}
	' "$tmp/pairs.s" || {
		printf "%s\n" "-O$level: unwanted FP moves, saves or destination in the pair fixture" >&2
		exit 1
	}
	"$asm" -c -o "$tmp/pairs.o" "$tmp/pairs.s"
done

# Check the order of the stateful operations, independently of registers.
for level in 0 2 3; do
	"$bin" -t ie64 -O"$level" -mfpext test/ie64-fpext/environment.ssa > "$tmp/env.s"
	awk '
		/^[a-z_]+:$/ { fn = substr($1, 1, length($1)-1) }
		$1 == ".size" { fn = "" }
		$1 ~ /^(fmovs[rc]|fmovc[rc]|frsqrt|fcvtds|fdiv|fcmp|fsincos|jsr|beq|bne)$/ {
			# Value comparisons emit their own status read and branches.
			if (fn == "ordinary" && $1 ~ /^(beq|bne)$/) next
			seq[fn] = seq[fn] " " $1
		}
		END {
			expect["status"] = " fmovsc frsqrt fmovsr fmovsc frsqrt fmovsr"
			expect["conditional"] = " fmovsc beq frsqrt fmovsr"
			expect["rounding"] = " fmovcr fmovcc fcvtds fmovcc fcvtds fmovcc"
			expect["literal_conversion"] = " fmovsc fcvtds fmovsr"
			expect["ordinary"] = " fmovsc fdiv fcmp fmovsr fmovsr fmovsc fdiv fcmp fmovsr fmovsr"
			expect["across_call"] = " frsqrt jsr frsqrt"
			expect["pair_reset"] = " fsincos fmovsc fsincos"
			for (fn in expect)
				if (seq[fn] != expect[fn]) {
					print fn ": expected" expect[fn] ", got" seq[fn] > "/dev/stderr"
					bad = 1
				}
			exit bad
		}
	' "$tmp/env.s"
	"$asm" -c -o "$tmp/env.o" "$tmp/env.s"
done

for level in 2 3; do
	"$bin" -t ie64 -O"$level" -mfpext -mfma test/ie64-fpext/inline.ssa > "$tmp/inline.s"
	awk '
		/^wrappers:$/ { body = 1 }
		$1 == ".size" { body = 0 }
		body && $1 ~ /^(jsr|jmp|fmin|frsqrt|fsincos|fround|fma)$/ { count[$1]++ }
		END {
			if (count["jsr"] || count["jmp"] || count["fmin"] != 1 ||
			    count["frsqrt"] != 1 || count["fsincos"] != 1 ||
			    count["fround"] != 1 || count["fma"] != 1) {
				print "expected each FP operation once in wrappers, with no calls" > "/dev/stderr"
				exit 1
			}
		}
	' "$tmp/inline.s"
	"$asm" -c -o "$tmp/inline.o" "$tmp/inline.s"
done

# Pair results stay live through a branch with a floating-point phi.
# The join must not rotate these registers through the stack.
for level in 0 2 3; do
	"$bin" -t ie64 -O"$level" -mfpext test/ie64-fpext/pair-join.ssa > "$tmp/pair-join.s"
	awk '
		$1 == "fsincos" { pair++ }
		$1 ~ /^(fload|fstore)$/ && /\(r31\)/ { stack++ }
		END { exit (pair != 1 || stack != 0) }
	' "$tmp/pair-join.s" || {
		printf "%s\n" "-O$level: stack traffic in the FSINCOS join" >&2
		exit 1
	}
	"$asm" -c -o "$tmp/pair-join.o" "$tmp/pair-join.s"
done
