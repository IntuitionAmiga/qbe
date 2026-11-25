#!/bin/sh
set -eu
qbe=${1:-./qbe}
asm=$("$qbe" -t ie64 -O2 test/ie64/extend-load.ssa)
printf "%s\n" "$asm" | awk '
    /^extend_half:|^extend_byte:|^reinterpret_half:/ { fn = $1 }
    /[[:space:]]sext[.][bwl][[:space:]]/ { ext[fn]++ }
    END {
        if (ext["extend_half:"] == 1 && ext["extend_byte:"] == 1 &&
            ext["reinterpret_half:"] == 1) exit 0
        print "unexpected sign extension count after a narrow load" > "/dev/stderr"
        exit 1
    }
'
