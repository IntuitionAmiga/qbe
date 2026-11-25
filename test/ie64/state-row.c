#include <stdint.h>
#include <stdio.h>

extern unsigned scan(const unsigned char *, unsigned *, unsigned);
extern unsigned choose(unsigned, unsigned *);
extern unsigned choose_many(unsigned, unsigned *);
extern unsigned row(const short *, const short *, unsigned, unsigned, unsigned, unsigned);

int main(void)
{
    unsigned n, result;
    static const short a[] = {-1, 3, 5, 7};
    static const short b[] = {11, 13, 17, 19, 23, 29};
    static const char chars[] = "abx";
    unsigned char input[6];

    for (unsigned k = 0; k < 243; k++) {
        for (unsigned alt = 0; alt < 2; alt++) {
            unsigned state = alt ? 9 : 3;
            unsigned saved = 41, pattern = k;
            for (unsigned i = 0; i < 5; i++) {
                unsigned ch = chars[pattern % 3];
                input[i] = ch;
                pattern /= 3;
                if (state == 3 && ch != 97) {
                    saved = ch;
                    state = 9;
                } else if (state == 9 && ch != 98)
                    state = 3;
            }
            input[5] = 0;
            n = 41;
            result = scan(input, &n, alt);
            if (result != state || n != saved)
                return 1;
        }
    }
    n = 41;
    if (choose(45, &n) != 7 || n != 42 || choose(46, &n) != 8 || n != 43)
        return 2;
    if (choose_many(45, &n) != 7 || n != 44
     || choose_many(43, &n) != 8 || n != 45
     || choose_many(0, &n) != 9 || n != 46)
        return 2;
    if (row(a, b, 1, 1, 3, 2) != 337
     || row(a, b, 1, 2, 3, UINT32_MAX) != 193
     || row(0, 0, UINT32_MAX, UINT32_MAX, 0, UINT32_MAX) != 0)
        return 3;
    puts("state and row checks passed");
    puts("IE_CM_DONE");
    return 0;
}
