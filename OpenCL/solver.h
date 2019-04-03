#ifndef SOLVER
#define SOLVER

#define GLOBAL_KERNELS 64

typedef struct {
    char data[64];
} block;

typedef struct {
    unsigned int h[4];
} hashes;

#endif
