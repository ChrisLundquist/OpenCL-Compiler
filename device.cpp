#include <iostream>
#include <CL/cl.h>
#include "device.h"
// return the device_ids for a target platform into the pointer passed
bool get_devices(cl_platform_id* target_platform, cl_device_id** devices, cl_uint* num_devices)
{
    // Make sure we can reallocate this
    if(devices == NULL)
        return false;
    // and this
    if(target_platform == NULL || *target_platform == NULL)
        return false;

    // query for number of devices
    if(CL_SUCCESS != clGetDeviceIDs(*target_platform,CL_DEVICE_TYPE_ALL, NULL,NULL, num_devices))
    {
        std::cerr << "Could not fetch the number of devices for target platform " << *target_platform << std::endl;
        return false;
    }
    std::cout << "Found " << *num_devices << " devices" << std::endl;

    // Allocate the memory to store the device ids
    *devices = new cl_device_id[*num_devices];

    // Query for device ids
    if(CL_SUCCESS != clGetDeviceIDs(*target_platform,CL_DEVICE_TYPE_ALL, *num_devices, *devices, NULL))
    {
        std::cerr << "Could not fetch the device ids" << std::endl;
        return false;
    }

    for( unsigned int i = 0; i < *num_devices; ++i){
        print_device_specs(*devices[i]);
    }

    return true;
}

bool print_device_specs(cl_device_id device)
{
    char* info = NULL;
    cl_uint* frequency = 0;
    size_t* max_work_item_sizes = NULL;
    cl_uint*  max_work_item_dimensions = NULL;

    info = (char*) get_device_info(device, CL_DEVICE_VENDOR);
    std::cout << "Device Vendor: " << info << std::endl;
    delete info;

    info = (char*) get_device_info(device, CL_DEVICE_NAME);
    std::cout << "Device Name: " << info << std::endl;
    delete info;

    frequency = (cl_uint*) get_device_info(device, CL_DEVICE_MAX_CLOCK_FREQUENCY );
    std::cout << "Device Clock Frequency: " << *frequency << std::endl;
    delete frequency;

    // Get the max dimensions so we know how many times to loop through sizes
    max_work_item_dimensions = (cl_uint*) get_device_info(device, CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);

    max_work_item_sizes = (size_t*) get_device_info(device, CL_DEVICE_MAX_WORK_ITEM_SIZES);
    std::cout << "Device Work Item Sizes: ";
    for(unsigned char i = 0; i < *max_work_item_dimensions; ++i)
         std::cout << max_work_item_sizes[i] << "x";
    std::cout << std::endl;

    delete max_work_item_dimensions;
    delete max_work_item_sizes;

    info = (char*) get_device_info(device, CL_DEVICE_EXTENSIONS);
    std::cout << "Device Extentions: " << info << std::endl;
    delete info;

    return true;
}


// The caller NEEDS to know the return type for the requested information
void* get_device_info(cl_device_id device, cl_uint info_property)
{
    cl_int status = 0;
    size_t info_size = 0;
    void *info = NULL;

    // See how much space we need for the "return" info
    status = clGetDeviceInfo(device,info_property, NULL, NULL, &info_size);
    if(CL_SUCCESS != status){
        if(CL_INVALID_DEVICE == status){
            std::cerr << "Couldn't get size of property for info_property " << info_property << ". Got CL_INVALID_DEVICE" << std::endl;
        }else if(CL_INVALID_VALUE == status)
            std::cerr << "Got CL_INVALID_VALUE when attempting to get info_property " << info_property << std::endl;
        return NULL;
    }

    // Allocate memory for the "return" info
    info = new char[info_size];

    if(info == NULL)
    {
        std::cerr << "Memory allocation for property info failed" << std::endl;
        return false;
    }
    
    status = clGetDeviceInfo(device, info_property, info_size, info, NULL);
    if(CL_SUCCESS != status){
        if(CL_INVALID_DEVICE == status){
            std::cerr << "Couldn't get size of property for info_property " << info_property << ". Got CL_INVALID_DEVICE" << std::endl;
        }else if(CL_INVALID_VALUE == status)
            std::cerr << "Got CL_INVALID_VALUE when attempting to get info_property " << info_property << std::endl;
        return NULL;
    }

    return info;
}
