#!/bin/sh
set -eu

qbe=${1:-./qbe}
section() {
    sed -n "/^$1:/,/^\.size $1/p"
}
affine=$("$qbe" -t ie64 -O2 -mbranch-imm test/ie64-loop-affine.ssa)
search=$("$qbe" -t ie64 -O2 -mbranch-imm test/ie64-loop-load.ssa)
types=$("$qbe" -t ie64 -O2 -mbranch-imm -fstrict-aliasing test/ie64-loop-types.ssa)
untyped=$("$qbe" -t ie64 -O2 -mbranch-imm test/ie64-loop-types.ssa)

if printf "%s\n" "$affine" | rg -q "^[[:space:]]+mul"; then
    echo "loop still multiplies its affine index" >&2
    exit 1
fi
find=$(printf "%s\n" "$search" | section find)
printf "%s\n" "$find" | rg -q "load\.b"
# The wide key load follows the entry null check and precedes the loop's
# byte load. The loop latch tests the loaded next pointer directly.
printf "%s\n" "$find" | awk '
    /beq/ && !key && !byte { guarded = 1 }
    /load[.]w/ { key = guarded && !byte }
    /load[.]b/ { byte = key }
    /bne/ && byte { latch = 1 }
    END { exit !(key && byte && latch) }
'
printf "%s\n" "$search" | section continuation | awk '
    /load[.]q/ { advance = 1 }
    /bne/ && advance { latch = 1 }
    END { exit !latch }
'
if printf "%s\n" "$search" | section same | rg -q "move\.l"; then
    echo "narrow equality still normalises both word operands" >&2
    exit 1
fi
# With alias information the second store is on the exit edge. Without
# it the store follows the read on every iteration, even if load
# elimination already keeps the old value in a temporary.
storeafter() {
    awk '/bhi/ { latch = 1 } /store[.]l/ && latch { store = 1 }
         END { exit !store }'
}
printf "%s\n" "$types" | section dot | storeafter
if printf "%s\n" "$untyped" | section dot | storeafter; then
    echo "untyped dot product moved its store" >&2
    exit 1
fi
if printf "%s\n" "$types" | section overlap | storeafter; then
    echo "a different store class incorrectly allowed promotion" >&2
    exit 1
fi
