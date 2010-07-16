#ifndef OPENCL_COMPILER_DEVICE_H
#define OPENCL_COMPILER_DEVICE_H
// return the device_ids for a target platform into the pointer passed
bool get_devices(cl_platform_id* target_platform, cl_device_id** devices, cl_uint* num_devices);

// The caller NEEDS to know the return type for the requested information
void* get_device_info(cl_device_id device, cl_uint info_property);

// Prints infomation to stdout about this device
bool print_device_specs(cl_device_id device);
#endif
