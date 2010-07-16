#include "platform.h"
#include <iostream> // std::cout, std::err, std::endl
//#include <stdio.h>

int get_platform_name( cl_platform_id* platform )
{
    // clGetPlatformInfo needs a place to put its name
    char platform_string[32];

    if(CL_SUCCESS == clGetPlatformInfo(*platform, CL_PLATFORM_NAME, 32, &platform_string, NULL))
    {
        std::cout << platform_string << std::endl;
    } else{
        std::cerr << "Couldn't get plaform info for " << platform << std::endl;
    }
    return CL_SUCCESS;
}

// Sets the passed pointer platforms to an array of all discovered platforms
int get_platforms( cl_platform_id** platforms, cl_uint* num_platforms)
{
    // Find the number of platforms
    if(CL_SUCCESS != clGetPlatformIDs(0, NULL, num_platforms))
    {
        std::cerr << "Error when asking information on OpenCL platforms" << std::endl;
        return false;
    }

    // Make sure they have at least one platform
    if(num_platforms == 0)
    {
        std::cerr << "No OpenCL platforms were found" << std::endl;
        return false;
    }

    // Tell them of our new found success! :)
    std::cout << "Found " << *num_platforms << " platform(s)" << std::endl;

    // Allocate memory to store our platform IDs
    *platforms = new cl_platform_id[*num_platforms];

    // Ask it to put the platforms in the space we provided
    if(CL_SUCCESS != clGetPlatformIDs(*num_platforms, *platforms, NULL))
        std::cerr << "Failed to get the the platform ids" << std::endl;

    // print the name of each of the detected platforms
    for(unsigned char i = 0; i < *num_platforms; ++i)
        get_platform_name(platforms[i]);

    return true;
}
