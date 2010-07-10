#include <iostream> // std::cout, std::err, std::endl
#include <stdio.h>
#include <CL/cl.h> // cl*
#include <getopt.h>


#define NVIDIA_PLATFORM "NVIDIA CUDA"
#define ATI_PLATFORM

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

    return true;
}

// as specified in OpenCL document section 4.3
// TODO: This is our responsibility to be thread safe
void log_context_errors(const char *errinfo, const void *private_info, size_t cb, void *user_data)
{
    std::cerr << *errinfo << std::endl;

    std::cout << private_info;
}

const char* read_source(const char* file_path)
{
    FILE * from = fopen(file_path, "r");
    char* buffer = NULL;

        if(!from)
        {
            std::cerr << "Couldn't open kernel source file of '" << file_path << "'" << std::endl;
            return false;
        }

        const unsigned int position_status = fseek(from, 0, SEEK_END);
        const unsigned long size = ftell(from);
        rewind(from);

        if(-1 == position_status || size == -1)
        {
            std::cerr << "Couldn't get positions in kernel source file" << std::endl;
            return false;
        }

        buffer = new char[size + 1];

        if(fread(buffer, 1, size, from) != size)
        {
            std::cerr << "An error occured when reading kernel source file" << std::endl;
            return false;
        }

        buffer[size] = '\0';

    fclose(from);

    return buffer;
}

int main(int argc, char** argv)
{
    // Platforms
    cl_platform_id* platforms = NULL;
    cl_uint num_platforms = 0;

    // Devices
    cl_device_id* devices = NULL;
    cl_uint num_devices = 0;

    // Contexts
    cl_int return_code = 0;
    cl_context compiler_context = NULL;

    // Programs
    cl_program target_program = NULL;

    if(!get_platforms(&platforms,&num_platforms))
        return false;
    // TODO filter platforms for any options passed

    // Get the devices for the platform
    // TODO: Get the devices for each platform
    if(!get_devices(platforms, &devices, &num_devices))
        return false;

    // Log any errors to std::error via log_context_errors()
    compiler_context = clCreateContext(NULL, num_devices, devices, log_context_errors, NULL, &return_code);
    if(return_code == CL_INVALID_DEVICE)
    {
        std::cerr << "Invalid device in opencl context creation" << std::endl;
        return false;
    }
    // Though documented this does not seem to be defined
    /*else if ( return_code == CL_INVALID_DEVICE_LIST) {
      std::cout << "This combination of devices cannot be used to create an opencl context" << std::endl;
      return false;
      } */
    else if (return_code == CL_DEVICE_NOT_AVAILABLE){
        std::cerr << "One of the devices is not available even though it may have been returned by clGetDeviceIDs()" << std::endl;
        return false;
    }else if(return_code == CL_OUT_OF_HOST_MEMORY)
    {
        std::cerr << "Host is out of memory" << std::endl;
        return false;
    }
    std::cout << "Created context" << std::endl;
    // TODO change this from argv1 to all of them
    const char* source = read_source(argv[1]);

    if(!source)
        return false;

    target_program = clCreateProgramWithSource(compiler_context, 1, &source, NULL, &return_code);

    // Check if something went wrong
    if(target_program == NULL)
    {
        if(return_code == CL_INVALID_CONTEXT)
        {
            std::cerr << "Invalid OpenCL Context passed when attempting to create a program from source" << std::endl;
            return false;
        }
        else if( return_code == CL_INVALID_VALUE)
        {
            std::cerr << "Invalid value passed when attempting to create a program from source" << std::endl;
            return false;
        }
        else if ( return_code == CL_COMPILER_NOT_AVAILABLE)
        {
            std::cerr << "Compiler not available to build the program from source" << std::endl;
            return false;
        }
        else if ( return_code == CL_OUT_OF_HOST_MEMORY)
        {
            std::cerr << "Host ran out of memory when attempting to build the target program" << std::endl;
            return false;
        }

    }
     if(CL_SUCCESS != clBuildProgram(target_program, num_devices, devices,/* OPTIONS */ NULL, NULL, NULL))
     {
        char buffer[4096];
        //FIXME Get the build log for all devices, not just the first
        int status = clGetProgramBuildInfo(target_program,devices[0], CL_PROGRAM_BUILD_STATUS,0,NULL,NULL);
        if(status == CL_BUILD_NONE)
        {
            std::cerr << "CL_BUILD_NONE was returned" << std::endl;
            return false;
        } else if (status == CL_BUILD_ERROR )
        {
            std::cerr << "CL_BUILD_ERROR\n\n" << std::endl;
        }
        else {
            // print the status so we can look at cl.h
            std::cout << "Error Code: " << status << std::endl;
        }

        clGetProgramBuildInfo(target_program, devices[0], CL_PROGRAM_BUILD_LOG, 4096, &buffer, NULL);
        
        for(unsigned int i = 0; i < 4096 && buffer[i]; ++i)
            std::cerr << buffer[i];
        return false;
     }
    // TODO change this from argv1 to all of them
    std::cout << "compiled " << argv[1] << " successfully" << std::endl;

    return 0;
}

