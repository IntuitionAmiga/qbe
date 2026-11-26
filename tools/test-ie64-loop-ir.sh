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

recurrence=$("$qbe" -t ie64 -O2 -mbranch-imm test/ie64-loop-recurrence.ssa)
for spec in shift_pair:w shift_single:q shift_narrow_step:w; do
    name=${spec%:*}
    width=${spec#*:}
    body=$(printf "%s\n" "$recurrence" | section "$name")
    if ! printf "%s\n" "$body" | rg -q "load\.$width[[:space:]]"; then
        echo "$name did not use a feedback table" >&2
        exit 1
    fi
    if printf "%s\n" "$body" | rg -q '^[[:space:]]+b[a-z]+[[:space:]]'; then
        echo "$name kept a loop branch" >&2
        exit 1
    fi
done
for name in signed_shift observed_step observed_counter; do
    printf "%s\n" "$recurrence" | section "$name" | rg -q '^[[:space:]]+bls[[:space:]]'
done
printf "%s\n" "$recurrence" | section observed_step | rg -q 'store\.l[[:space:]]'
twice=$(printf "%s\n" "$recurrence" | section shift_twice)
if [ "$(printf "%s\n" "$twice" | rg -c 'lo32\(\.L__ie64_bits_')" -ne 1 ]; then
    echo "shift_twice materialised the table address twice" >&2
    exit 1
fi
