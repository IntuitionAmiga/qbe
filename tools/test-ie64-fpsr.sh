#!/bin/sh
# SPDX-FileCopyrightText: 2026 Zayn Otley
# SPDX-License-Identifier: LicenseRef-IntuitionEngine-Proprietary
set -eu

bin=${1:-./qbe}
asm=${IE64ASM:-../IntuitionEngine/sdk/bin/ie64asm}
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' EXIT HUP INT TERM

failed=0
for level in 0 2 3; do
	"$bin" -t ie64 -O"$level" -mfpext test/ie64-fpext/observers.ssa > "$tmp/fpsr.s"
	"$asm" -c -o "$tmp/fpsr.o" "$tmp/fpsr.s"
	if ! awk -v level="$level" '
		/^[a-z_]+:$/ { fn = substr($1, 1, length($1)-1) }
		$1 == ".size" { fn = "" }
		fn == "" { next }
		$1 == "fmovsr" && $2 == "r30" { next }
		$1 == "fmovsr" && $2 == "r29" {
			guards[fn]++
			saved = 1
			if (fn ~ /^switch_/) dispatch[fn] = dispatch[fn] " save"
			next
		}
		$1 == "fmovsc" && $2 == "r29" {
			saved = 0
			if (fn ~ /^switch_/) dispatch[fn] = dispatch[fn] " restore"
			next
		}
		fn ~ /^switch_/ {
			if ($1 == "bls") dispatch[fn] = dispatch[fn] " bounds"
			if ($1 == "lsl.q" && $2 == "r29,") dispatch[fn] = dispatch[fn] " index"
			if ($1 == "jmp" && $2 == "(r30)") dispatch[fn] = dispatch[fn] " jump"
			if ($1 == "dload") dispatch[fn] = dispatch[fn] " load"
			if ($1 ~ /^(fmovi|fmovecr)$/) dispatch[fn] = dispatch[fn] " constant"
		}
		$1 ~ /^(fmovsc|fmovsr|fdiv|fneg|fcvtsd|fcvtds|dcvtfi|dsincos|dsin|jsr|bra)$/ {
			if ($1 == "jsr" || ($1 == "bra" && $2 ~ /^observe/))
				seq[fn] = seq[fn] " call"
			else if ($1 != "bra")
				seq[fn] = seq[fn] " " $1
		}
		fn == "pair_pressure" && $1 == "dsincos" { in_pair = 1 }
		fn == "pair_pressure" && $1 == "fmovsr" { in_pair = 0 }
		fn == "pair_pressure" && $1 == "dload" && in_pair { clobber[fn]++ }
		fn == "transport" && $1 == "dsin" { in_transport = 1 }
		fn == "transport" && $1 == "fmovsr" { in_transport = 0 }
		in_transport && $1 ~ /^(dload|fmovi|fmovecr)$/ {
			transport++
			if (!saved) clobber[fn]++
		}
		fn == "call_argument" && $1 ~ /^(fmovi|fmovecr)$/ {
			argument++
			if (!saved) clobber[fn]++
		}
		END {
			expect["unused_div"] = " fmovsc fdiv fmovsr"
			expect["sticky"] = " fmovsc fdiv fneg fmovsr"
			expect["cleared"] = " fmovsc fmovsr"
			expect["unobserved"] = ""
			expect["unused_wide"] = ""
			expect["unused_precision"] = ""
			expect["incoming"] = " fdiv call"
			expect["edge"] = " fmovsc fneg fdiv fmovsr"
			expect["pair_pressure"] = " dsincos fmovsr"
			expect["transport"] = " dsin fmovsr"
			expect["call_argument"] = " fdiv call"
			expect["literal_signed"] = " fmovsc fcvtsd dcvtfi fmovsr"
			expect["literal_single_unsigned"] = " fmovsc fcvtsd dcvtfi fmovsr"
			expect["literal_double_unsigned"] = " fmovsc dcvtfi fmovsr"
			expect["literal_unobserved"] = ""
			expect["literal_single_word"] = " fmovsc fcvtsd dcvtfi fmovsr"
			expect["literal_double_long"] = " fmovsc dcvtfi fmovsr"
			expect["loop"] = " fmovsc fmovsr fdiv"
			expect["switch_case"] = " fmovsc fdiv fmovsr"
			expect["switch_default"] = " fmovsc fdiv fneg fmovsr"
			expect["switch_backedge"] = " fmovsc fmovsr fdiv"
			expect["switch_phi"] = " fmovsc fdiv fmovsr"
			expect["switch_cleared"] = " fmovsc fmovsr"

			# Restore before the bounds branch and before the index overwrites r29.
			jump = " bounds index jump"
			path["switch_case"] = " save load restore" jump
			path["switch_default"] = jump
			path["switch_backedge"] = jump
			path["switch_phi"] = jump " save constant restore"
			path["switch_cleared"] = jump
			for (fn in path)
				if (dispatch[fn] != path[fn]) {
					printf "-O%s %s: expected%s, got%s\n", level, fn,
						path[fn], dispatch[fn] > "/dev/stderr"
					bad = 1
				}
			if (seq["edge"] == " fmovsc fdiv fneg fmovsr")
				seq["edge"] = expect["edge"]
			for (fn in expect)
				if (seq[fn] != expect[fn] || clobber[fn]) {
					printf "-O%s %s: expected%s, got%s, clobbers %d\n", level, fn,
						expect[fn], seq[fn], clobber[fn] > "/dev/stderr"
					bad = 1
				}
						if (guards["transport"] != 1 || !transport || !argument) {
				printf "-O%s: transport guards %d, transports %d, arguments %d\n",
					level, guards["transport"], transport, argument > "/dev/stderr"
				bad = 1
			}
			exit bad
		}
	' "$tmp/fpsr.s"; then
		failed=1
	fi
done
exit "$failed"
