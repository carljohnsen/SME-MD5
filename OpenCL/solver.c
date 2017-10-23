#include <stdio.h>
#include <stdlib.h>
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#include <CL/cl.h>
#include "solver.h"
#include <time.h>

int main() {
    cl_device_id device_id;
    cl_context context;
    cl_command_queue command_queue;
    cl_mem memoutput;
    cl_mem memr;
    cl_mem memkk;
    cl_mem memtargets;
    cl_mem memsteps;
    cl_program program;
    cl_kernel kernel;
    cl_uint num_devices;
    cl_uint num_platforms;
    cl_int ret;

    clock_t start, diff;
    FILE *fp;
    char filename[] = "./hello.cl";
    char *source_str;
    size_t source_size;

    fp = fopen(filename, "r");
    if (!fp) {
        printf("Failed to load kernel.\n");
        return(1);
    }

    source_str = (char*) malloc(4096);
    source_size = fread(source_str, 1, 4096, fp);
    fclose(fp);

    cl_platform_id *platforms = (cl_platform_id *) malloc(sizeof(cl_platform_id) * 2);
    ret = clGetPlatformIDs(2, platforms, &num_platforms);
    // 0 = intel, 1 = nvidia
    ret = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 1, &device_id, &num_devices);

    context = clCreateContext(NULL, 1, &device_id, NULL, NULL, &ret);
    command_queue = clCreateCommandQueue(context, device_id, 0, &ret);

    unsigned int r[] = {
            7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
            5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
            4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
            6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};
    unsigned int kk[] = {
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
    unsigned int targets[] = {
        //0xd9edb07b, 0x0a43228f, 0x857fb603, 0xcac2833e // '        '
        //0x77014ee7, 0x9d736239, 0xabab58cb, 0x0617a0b1 // '       A'
        //0x4751d31e, 0x52f30fdd, 0x0208f3ea, 0x9be1aa90 // '     AAA'
        0x3fe3ee64, 0x19b6ba7c, 0x9ba61a42, 0xfe326560 // 'AA      '
        //0x8f4b1921, 0x98537d1a, 0x3cf99610, 0x2a427486 // 'AAA     '
        //0x98a4586f, 0x7f2feac3, 0xe26eaa22, 0x323d3552 // 'AAAA    '
        //0x8ce3e9ae, 0xc20ed4b4, 0x56424579, 0xc8b43975 // 'AAAAAAAA'
    };
    unsigned int tmp_output[] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    memoutput = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 8 * sizeof(int), tmp_output, &ret);
    memr = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 64 * sizeof(int), r, &ret);
    memkk = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 64 * sizeof(int), kk, &ret);
    memtargets = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 4 * sizeof(int), targets, &ret);
    memsteps = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(long), NULL, &ret);

    program = clCreateProgramWithSource(context, 1, (const char**) &source_str, (const size_t *) &source_size, &ret);
    ret = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    printf("--- %d\n", ret);
    if (ret == CL_BUILD_PROGRAM_FAILURE) {
        // Determine the size of the log
        size_t log_size;
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

        // Allocate memory for the log
        char *log = (char *) malloc(log_size);

        // Get the log
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

        // Print the log
        printf("%s\n", log);
        exit(1);
    }

    kernel = clCreateKernel(program, "md5_const", &ret);
    printf("--- %d\n", ret);
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *) &memoutput);
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *) &memr);
    ret = clSetKernelArg(kernel, 2, sizeof(cl_mem), (void *) &memkk);
    ret = clSetKernelArg(kernel, 3, sizeof(cl_mem), (void *) &memtargets);
    ret = clSetKernelArg(kernel, 4, sizeof(cl_mem), (void *) &memsteps);

    size_t global_size = GLOBAL_KERNELS;
    size_t local_size = 64;
    unsigned long steps = 0;
    unsigned int output[8];
    start = clock();
    for (int i = 0; i < 10000; i++) {
        ret = clEnqueueWriteBuffer(command_queue, memsteps, CL_TRUE, 0, sizeof(long), &steps, 0, NULL, NULL);
        if (ret != 0) printf("%d\n", ret);
        ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size,
             &local_size, 0, NULL, NULL);
        if (ret != 0) printf("%d\n", ret);
        ret = clEnqueueReadBuffer(command_queue, memoutput, CL_TRUE, 0, 8 * sizeof(int), output, 0, NULL, NULL);
        if (ret != 0) printf("%d\n", ret);
        steps += KERNEL_ITERATIONS;
        if (output[0] == 1) break;
    }
    diff = clock() - start;
    float secs = diff * 1.0 / CLOCKS_PER_SEC;
    long guesses = global_size * steps;
    printf("Made %lu M guesses in %.02f seconds | hashrate = %.02f MHash\n", guesses/1000000, secs, guesses/secs/1000000);

    printf("--- %d\n", ret);
    for (int i = 0; i < 5; i++) {
        printf("0x%08x ", output[i]);
    }
    printf("'%s'\n", (char*) (output + 5));

    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(memoutput);
    ret = clReleaseMemObject(memr);
    ret = clReleaseMemObject(memkk);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);
    free(source_str);

    return 0;
}
