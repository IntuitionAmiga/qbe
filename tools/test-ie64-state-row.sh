#!/bin/sh
set -eu
qbe=${1:-./qbe}
test="${2:-all}"

if [ "$test" = all ] || [ "$test" = state ]; then
    asm=$("$qbe" -t ie64 -O2 -mbranch-imm test/ie64/loop-state.ssa)
    printf "%s\n" "$asm" | awk '
        /[[:space:]]b(eq|ne)[[:space:]].*, #3,/ { n++ }
        END { if (n) print "state dispatch remains" > "/dev/stderr"; exit !!n }
    '
fi
if [ "$test" = all ] || [ "$test" = merge ]; then
    asm=$("$qbe" -t ie64 -O2 -mbranch-imm test/ie64/merge-branches.ssa)
    printf "%s\n" "$asm" | awk '
        /^choose(_many)?:/ { fn = $1 }
        /[[:space:]]store[.]l[[:space:]]/ { n[fn]++ }
        END {
            if (n["choose:"] == 1 && n["choose_many:"] == 1) exit 0
            print "duplicated store remains" > "/dev/stderr"
            exit 1
        }
    '
fi
if [ "$test" = all ] || [ "$test" = row ]; then
    ir=$("$qbe" -t ie64 -O2 -mbranch-imm -dG test/ie64/loop-row.ssa 2>&1)
    printf "%s\n" "$ir" | awk '
        /^> After GCM:/ { w = l = stale = 0; fast = 0 }
        /^@/ { fast = $1 == "@body.row" }
        fast && /=w add .*%stride$/ { stale++ }
        /row.* =l phi/ { l++ }
        /stride.* =w add/ { w++ }
        END {
            if (l < 2 || !w) print "missing full-width row recurrences or word fallback" > "/dev/stderr"
            if (stale) print "unused word recurrence remains in the fast loop" > "/dev/stderr"
            exit l < 2 || !w || stale
        }
    '
fi
