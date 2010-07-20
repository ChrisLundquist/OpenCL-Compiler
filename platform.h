#ifndef OPENCL_COMPILER_PLATFORM_H
#define OPENCL_COMPILER_PLATFORM_H

#include <CL/cl.h> // cl*
#define NVIDIA_PLATFORM "NVIDIA CUDA"
#define ATI_PLATFORM "ATI Stream"

// Prints the platform name to std out
int print_platform_name( cl_platform_id platform );

// Sets the passed pointer platforms to an array of all discovered platforms
int get_platforms( cl_platform_id** platforms, cl_uint* num_platforms);
#endif
