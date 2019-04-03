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
    cl_mem meminput;
    cl_mem memoutput;
    cl_program program;
    cl_kernel kernel;
    cl_uint num_devices;
    cl_uint num_platforms;
    cl_int ret;

    clock_t start, diff;
    FILE *fp;
    char filename[] = "./solver.cl";
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

    block tmp_input;
    hashes tmp_output;
    meminput = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(block), &tmp_input, &ret);
    memoutput = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(hashes), &tmp_output, &ret);

    program = clCreateProgramWithSource(context, 1, (const char**) &source_str, (const size_t *) &source_size, &ret);
    ret = clBuildProgram(program, 1, &device_id, "-I ./", NULL, NULL);
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

    kernel = clCreateKernel(program, "md5", &ret);
    printf("--- %d\n", ret);
    ret = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *) &meminput);
    ret = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void *) &memoutput);

    size_t global_size = GLOBAL_KERNELS;
    size_t local_size = 64;
    for (int i = 0; i < 4; i++) tmp_input.data[i] = 0x42;
    tmp_input.data[4] = 128;
    for (int i = 5; i < 56; i++) tmp_input.data[i] = 0;
    tmp_input.data[56] = 4*8;
    for (int i = 57; i < 64; i++) tmp_input.data[i] = 0;

    start = clock();
    //for (int i = 0; i < 10000; i++) {
        ret = clEnqueueWriteBuffer(command_queue, meminput, CL_TRUE, 0, sizeof(block), &tmp_input, 0, NULL, NULL);
        if (ret != 0) printf("a %d\n", ret);
        ret = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, &global_size, &local_size, 0, NULL, NULL);
        if (ret != 0) printf("b %d\n", ret);
        ret = clEnqueueReadBuffer(command_queue, memoutput, CL_TRUE, 0, sizeof(hashes), &tmp_output, 0, NULL, NULL);
        if (ret != 0) printf("c %d\n", ret);
    //}
    diff = clock() - start;
    float secs = diff * 1.0 / CLOCKS_PER_SEC;
    printf("Made %d M guesses in %.02f seconds\n", 1, secs);

    printf("--- %d\n", ret);
    char *ch = (char *)tmp_output.h;
    for (int i = 0; i < 16; i++)
        printf("%02x", ch[i] & 0xFF);

    ret = clFlush(command_queue);
    ret = clFinish(command_queue);
    ret = clReleaseKernel(kernel);
    ret = clReleaseProgram(program);
    ret = clReleaseMemObject(meminput);
    ret = clReleaseMemObject(memoutput);
    ret = clReleaseCommandQueue(command_queue);
    ret = clReleaseContext(context);
    free(source_str);

    return 0;
}
