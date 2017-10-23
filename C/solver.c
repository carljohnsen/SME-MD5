#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#define CORES 8
#define SEARCH_SPACE 0xFFFFFFFF00000000
#define STEP_SIZE SEARCH_SPACE / CORES

int core(char *input, unsigned int *targets);
long generator(int index, int *valid, char *result, unsigned int *targets);
void top_level(unsigned int hashes[4], char output[8]);
int verifier(unsigned int *hashes, unsigned int *targets);

int main(int argc, char **argv) {
    int solvers = 1, valid = 0;
    unsigned int targets[] = {
        //0x8f4b1921, 0x98537d1a, 0x3cf99610, 0x2a427486 // 'AAA     '
        0x98a4586f, 0x7f2feac3, 0xe26eaa22, 0x323d3552 // 'AAAA    '
    };
    char result[8];
    switch (argc) {
        case 5: targets[0] = (unsigned int) strtol(argv[1], NULL, 0);
                targets[1] = (unsigned int) strtol(argv[2], NULL, 0);
                targets[2] = (unsigned int) strtol(argv[3], NULL, 0);
                targets[3] = (unsigned int) strtol(argv[4], NULL, 0);
        case 1: break;
        default: printf("Usage: %s [targets: {h0, h1, h2, h3}]", argv[0]);
    }
    struct timeval start, end;
    unsigned long guesses = 0;
    int c;
    gettimeofday(&start, NULL);
#pragma omp parallel for private(c) shared(valid) schedule(dynamic) reduction(+:guesses)
    for (c = 0; c < CORES; c++) {
        guesses += generator(c, &valid, result, targets);
    }
    gettimeofday(&end, NULL);
    unsigned long sec = end.tv_sec - start.tv_sec;
    unsigned long msec = (end.tv_usec - start.tv_usec) / 1000;
    unsigned long total_msec = sec * 1000 + msec;
    unsigned long guesses_msec = guesses / total_msec;
    double hashrate = (0.0 + guesses_msec) / 1000;
    printf("--'%s'--\n", result);
    printf("%luM guesses in %u sec %u ms = %.02f MHash\n", guesses/1000000, sec, msec, hashrate);
}

int core(char *input, unsigned int *targets) {
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
    unsigned int *w = (unsigned int *) input;
    unsigned int a = h0;
    unsigned int b = h1;
    unsigned int c = h2;
    unsigned int d = h3;

    for (int i = 0; i < 64; i++) {
#pragma HLS PIPELINE
        unsigned int f, g;
        if (i < 16) {
            f = (b & c) | ((~b) & d);
            g = i;
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
        unsigned int x = a + f + k[i] + w[g];
        int c2 = (int) r[i];
        b = b + (((x) << (c2)) | ((x) >> (32 - (c2))));
        a = temp;
    }

    unsigned int hashes[4];
    hashes[0] = h0 + a;
    hashes[1] = h1 + b;
    hashes[2] = h2 + c;
    hashes[3] = h3 + d;
    return verifier(hashes, targets);
}

long generator(int index, int *valid, char *result, unsigned int *targets) {
    unsigned long steps = 0;
    char input[64];
    int i;
    for (i = 0; i < 64; i++) input[i] = 0;
    unsigned long remainder = STEP_SIZE * index;
    unsigned int base = 127-32;
    for (i = 0; i < 8; i++) {
        input[i] = (remainder % base) + 32;
        remainder = remainder / base;
    }
    input[8] = 128;
    input[56] = 64;
    while (1) {
        if (*(valid) == 1) break;
        steps++;
        if (core(input, targets)) {
            *(valid) = 1;
            for (int i = 0; i < 8; i++) {
                result[i] = input[i];
            }
            break;
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
    return steps;
}

void top_level(unsigned int hashes[4], char output[8]) {
	int valid = 0;
	generator(0, &valid, output, hashes);

}

int verifier(unsigned int *hashes, unsigned int *targets) {
    return hashes[0] == targets[0] &&
        hashes[1] == targets[1] &&
        hashes[2] == targets[2] &&
        hashes[3] == targets[3];
}
