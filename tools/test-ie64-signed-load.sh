#!/bin/sh
set -eu
qbe=${1:-./qbe}
src=test/ie64/signed-load.ssa.compile
off=$("$qbe" -t ie64 -O2 "$src")
on=$("$qbe" -t ie64 -O2 -msigned-loads "$src")
printf '%s\n' "$off" | awk '
    /^[[:alnum:]_]+:/ { fn = $1 }
    /[[:space:]]load[.]s[bwl][[:space:]]/ { exit 1 }
    /[[:space:]]sext[.][bwl][[:space:]]/ { ext[fn]++ }
    END { if (ext["signed_byte:"] != 1 || ext["signed_half:"] != 1 ||
        ext["signed_word:"] != 1) exit 1 }
'
printf '%s\n' "$on" | awk '
    /^[[:alnum:]_]+:/ { fn = $1 }
    /[[:space:]]load[.]s[bwl][[:space:]]/ { load[fn, $1]++ }
    /[[:space:]]sext[.][bwl][[:space:]]/ { ext[fn]++ }
    END {
        if (load["signed_byte:", "load.sb"] != 1 ||
            load["signed_half:", "load.sw"] != 1 ||
            load["signed_word:", "load.sl"] != 1 ||
            load["volatile_half:", "load.sw"] != 1 || ext["volatile_half:"] ||
            load["unsigned_word:", "load.sl"] || load["word_result:", "load.sl"] ||
            ext["signed_byte:"] || ext["signed_half:"] || ext["signed_word:"] ||
            ext["unsigned_word:"] || ext["word_result:"] != 1) exit 1
    }
'
if "$qbe" -t amd64_sysv -msigned-loads "$src" >/dev/null 2>&1; then
    echo '-msigned-loads accepted on a non-IE64 target' >&2
    exit 1
fi
if [ -n "${IE64ASM:-}" ]; then
    tmp=$(mktemp -d)
    trap 'rm -rf "$tmp"' EXIT HUP INT TERM
    printf '%s\n' "$on" > "$tmp/signed-load.s"
    "$IE64ASM" -c -Werror -o "$tmp/signed-load.o" "$tmp/signed-load.s"
fi
