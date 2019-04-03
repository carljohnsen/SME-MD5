#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <openssl/md5.h>

#define BLOCK_SIZE_BITS 512
#define BLOCK_SIZE BLOCK_SIZE_BITS / 8
#define KB 1024
#define MB KB * 1024
#define GB MB * 1024
#define NUM_BLOCKS (1 * GB) / BLOCK_SIZE

typedef struct {
    char data[BLOCK_SIZE];
} block;

typedef struct {
    unsigned int h[4];
} hashes;

hashes core(block input);
void tester();

int main(int argc, char **argv) {
    tester();
}

hashes core(block input) {
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

    unsigned int *w = (unsigned int *) input.data;

    for (int i = 0; i < 64; i++) {
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
        b = b + ((x << r[i]) | (x >> (32 - r[i])));
        a = temp;
    }

    hashes result;
    result.h[0] = h0 + a;
    result.h[1] = h1 + b;
    result.h[2] = h2 + c;
    result.h[3] = h3 + d;
    return result;
}

void tester() {
    unsigned long total_usec = 0;
    srand(time(NULL));

    // Allocate the blocks and the results
    block blocks[NUM_BLOCKS];
    hashes results[NUM_BLOCKS];
    int sizes[NUM_BLOCKS];
    for (int i = 0; i < NUM_BLOCKS; i++) {
        // Generate the data
        sizes[i] = rand() % 13;
        int tmp[16];
        for (int j = 0; j < sizes[i]; j++)
            tmp[j] = rand();
        tmp[sizes[i]] = 128;
        for (int j = sizes[i]+1; j < 15; j++)
            tmp[j] = 0;
        tmp[14] = sizes[i] * 4 * 8;
        tmp[15] = 0;
        
        // Fill the block
        for (int j = 0; j < 16; j++)
            ((int *)blocks[i].data)[j] = tmp[j];
    }

    // Set up timing
    struct timeval start, end;
    unsigned long guesses = 0;
    gettimeofday(&start, NULL);

    for (int i = 0; i < NUM_BLOCKS; i++) {
        // Compute the resulting hash
        results[i] = core(blocks[i]);
    }

    // Stop timing
    gettimeofday(&end, NULL);
    unsigned long usec = end.tv_usec - start.tv_usec;
    total_usec += usec;

    for (int i = 0; i < NUM_BLOCKS; i++) {
        // Compute the library hash for verification
        unsigned char digest[16];
        MD5((unsigned char *)blocks[i].data, sizes[i] * 4, digest);

        // Compare the outputs
        int eq = 1;
        for (int j = 0; j < 16; j++)
            eq = eq && digest[j] == ((unsigned char *)results[i].h)[j];

        // Report error, if any
        if (!eq) {
            // Print the digests
            printf("Not equal\n");
            for (int j = 0; j < 4; j++)
                printf("%08X ", ((int*)digest)[j]);
            printf(" - verified\n");
            for (int j = 0; j < 4; j++)
                printf("%08X ", results[i].h[j]);
            printf(" - computed\n");
        }
    }
                  usec =  total_usec % 1000;
    unsigned long msec = (total_usec / 1000) % 1000;
    unsigned long  sec = (total_usec / 1000) / 1000;
    printf("Test done. Core finished in %lu.%03lu%03lu seconds\n", sec, msec, usec);
    printf("That is %lu usec per block", total_usec / NUM_BLOCKS);
}
