#include "solver.h"

__kernel void md5_const(__global uint* output, __constant uint* r, __constant uint*kk,
                           __constant uint* targets, __constant ulong* steps) {
    __private char input[64];
    __private uint *w;
    __private uint h0, h1, h2, h3, a, b, c, d, f, g, tmp, x;
    __private int c2;

    uint loc_addr = get_local_id(0);
    uint global_addr = get_global_id(0);

    ulong offset = GENERATOR_STEP_SIZE * global_addr;
    ulong remainder = offset + steps[0];
    uint base = 127-32;
    for (int i = 0; i < 8; i++) {
        input[i] = (remainder % base) + 32;
        remainder = remainder / base;
    }

    input[8] = 128;
    input[56] = 64;
    w = (uint*) input;

    for (int j = 0; j < KERNEL_ITERATIONS; j++) {
        h0 = 0x67452301;
        h1 = 0xefcdab89;
        h2 = 0x98badcfe;
        h3 = 0x10325476;

        a = h0;
        b = h1;
        c = h2;
        d = h3;

        for (int i = 0; i < 64; i++) {
            if (i < 16) {
                f = (b & c) | ((~b) & d);
                g = (uint)i;
            } else if (i < 32) {
                f = (d & b) | ((~d) & c);
                g = (uint)(5 * i + 1) % 16;
            } else if (i < 48) {
                f = b ^ c ^ d;
                g = (uint)(3 * i + 5) % 16;
            } else {
                f = c ^ (b | (~d));
                g = (uint)(7 * i) % 16;
            }

            tmp = d;
            d = c;
            c = b;
            x = a + f + kk[i] + w[g];
            c2 = (int)r[i];
            b = b + (((x) << (c2)) | ((x) >> (32 - (c2))));
            a = tmp;
        }

        h0 += a;
        h1 += b;
        h2 += c;
        h3 += d;

        if (h0 == targets[0] && h1 == targets[1] &&
               h2 == targets[2] && h3 == targets[3]) {
            output[0] = 1;
            output[1] = h0;
            output[2] = h1;
            output[3] = h2;
            output[4] = h3;
            output[5] = w[0];
            output[6] = w[1];
            return;
        } else {
            for (int i = 0; i < 8; i++) {
                input[i] = (input[i] + 1) % 127;
                if (input[i] == 0) {
                    input[i] = 32;
                } else {
                    break;
                }
            }
        }
    }
    barrier(CLK_GLOBAL_MEM_FENCE);
}

