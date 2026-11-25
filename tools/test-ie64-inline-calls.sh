#!/bin/sh
set -eu
qbe=${1:-./qbe}
asm=$("$qbe" -t ie64 -O2 test/ie64/inline-calls.ssa)
printf "%s\n" "$asm" | awk -v only="${2:-all}" '
    /^[[:alpha:]_][[:alnum:]_]*:$/ { fn = $1 }
    /[[:space:]](jsr|bra)[[:space:]]/ {
        if ($2 !~ /^\.L/) { calls[fn ":" $2]++; total[fn]++ }
    }
    END {
        callback = total["known:"] || !calls["dynamic::walk"] ||
            !calls["escaped::mutable"] || !calls["known_reassigned::reassigned"]
        wrapper = calls["use_forward::forward"] || !calls["use_forward::external"]
        direct = !calls["known_nonleaf::nonleaf"] || total["known_nonleaf:"] != 1 ||
            !calls["external_callback::walk"]
        if (only == "wrapper" || only == "direct") callback = 0
        if (only == "callback" || only == "direct") wrapper = 0
        if (only == "wrapper" || only == "callback") direct = 0
        if (callback) print "known callback was not inlined, or a dynamic path changed" > "/dev/stderr"
        if (wrapper) print "forwarding wrapper was not inlined" > "/dev/stderr"
        if (direct) print "directisation changed an unresolved target or missed a known non-leaf" > "/dev/stderr"
        exit callback || wrapper || direct
    }
'
