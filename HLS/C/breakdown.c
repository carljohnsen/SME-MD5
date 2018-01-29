#include "ap_cint.h"

#define CORES 8
#define SEARCH_SPACE 0xFFFFFFFF00000000
#define STEP_SIZE SEARCH_SPACE / CORES

typedef uint1 bool;

typedef struct {
	unsigned int h0;
	unsigned int h1;
	unsigned int h2;
	unsigned int h3;
} hashes_t;

typedef struct {
	unsigned int valid;
	unsigned long long string;
} top_output_t;

void init_generator(int index, char output[8]) {
    unsigned long remainder = STEP_SIZE * index;
    unsigned int base = 127-32;
    for (int i = 0; i < 8; i++) {
#pragma HLS PIPELINE II=1
        output[i] = (remainder % base) + 32;
        remainder = remainder / base;
    }
}

void inc_generator(char str[8]) {
	for (int i = 0; i < 8; i++) {
#pragma HLS PIPELINE II=1
		str[i] = (char) ((uint7) (str[i] + 1));
		if (str[i] == 0) {
			str[i] = 32;
		} else {
			break;
		}
	}
}

void packer(char input[8], unsigned int output[16]) {
	char tmp[64];
	for (int i = 0; i < 8; i++) {
#pragma HLS UNROLL
		tmp[i] = input[i];
	}
	tmp[8] = 128;
	for (int j = 9; j < 56; j++) {
#pragma HLS UNROLL
		tmp[j] = 0;
	}
	tmp[56] = 64;
	for (int k = 56; k < 64; k++) {
#pragma HLS UNROLL
		tmp[k] = 0;
	}
	for (int h = 0; h < 16; h++) {
#pragma HLS UNROLL
		output[h] = tmp[(h << 2) + 0] |
				tmp[(h << 2) + 1] << 8 |
				tmp[(h << 2) + 2] << 16 |
				tmp[(h << 2) + 3] << 24;
	}
}

void core(unsigned int input[16], hashes_t *output) {
    unsigned int r[] = {
        7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
        5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
        4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
        6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};
    unsigned int k[] = {
        0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
        0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
        0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
        0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
        0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
        0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
        0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
        0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
        0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
        0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
        0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
        0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
        0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
        0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
        0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
        0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};
    unsigned int h0 = 0x67452301;
    unsigned int h1 = 0xefcdab89;
    unsigned int h2 = 0x98badcfe;
    unsigned int h3 = 0x10325476;
    unsigned int a = h0;
    unsigned int b = h1;
    unsigned int c = h2;
    unsigned int d = h3;
    for (int i = 0; i < 64; i++) {
#pragma HLS PIPELINE II=15
        unsigned int f, g;
        if (i < 16) {
            f = (b & c) | ((~b) & d);
            g = i;unsigned int h0;
        } else if (i < 32) {
            f = (d & b) | ((~d) & c);
            g = (5*i + 1) % 16;
        } else if (i < 48) {
            f = b ^ c ^ d;
            g = (3*i + 5) % 16;
        } else {
            f = c ^ (b | (~d));
            g = (7*i) % 16;
        }
        unsigned int temp = d;
        d = c;
        c = b;
        unsigned int x = a + f + k[i] + input[g];
        int c2 = (int) r[i];
        b = b + (((x) << (c2)) | ((x) >> (32 - (c2))));
        a = temp;
    }
    output->h0 = h0 + a;
    output->h1 = h1 + b;
    output->h2 = h2 + c;
    output->h3 = h3 + d;
}

void verifier(hashes_t *target, hashes_t *guess, int *output) {
	*output = target->h0 == guess->h0 &&
			target->h1 == guess->h1 &&
			target->h2 == guess->h2 &&
			target->h3 == guess->h3;
}

void solver(hashes_t target, unsigned long long *out_str, bool *out_valid) {
//#pragma HLS DATAFLOW
//#pragma HLS INTERFACE axis port=target
//#pragma HLS INTERFACE axis port=out_str
//#pragma HLS INTERFACE axis port=out_valid
#pragma HLS INTERFACE ap_ctrl_none port=return
	char str[8];
	init_generator(0, str);
	while (true) {
//#pragma HLS DATAFLOW
#pragma HLS PIPELINE
		unsigned int tmp[16];
		packer(str, tmp);
		hashes_t guess;
		core(tmp, &guess);
		int valid;
		verifier(&target, &guess, &valid);
		if (valid) {
			*out_str = *((unsigned long long*) str);
			*out_valid = (bool) valid;
			return;
		}
		inc_generator(str);
	}
}
