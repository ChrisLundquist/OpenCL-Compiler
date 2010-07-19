#include <iostream> // std::cout, std::err, std::endl
#include <stdio.h>
#include <CL/cl.h> // cl*
#include "platform.h"
#include "device.h"
#include <unistd.h>
#include <string>
#ifndef _WIN32
#include <getopt.h> // TODO: Add getopt options for Linux and Windows
#endif


// as specified in OpenCL document section 4.3
// TODO: This is our responsibility to be thread safe
#ifdef _WIN32
void __stdcall log_context_errors(const char *errinfo, const void *private_info, size_t cb, void *user_data)
#else
void log_context_errors(const char *errinfo, const void *private_info, size_t cb, void *user_data)
#endif
{
    std::cerr << errinfo << std::endl;
    for(unsigned int i = 0; i < cb; i++)
    {
        std::cout << ((char*)private_info)[i];
    }
    std::cout << std::endl;

    //FIXME what is user_data?
    for(unsigned int i = 0; i < cb; i++)
    {
        std::cout << ((char*)user_data)[i];
    }
    std::cout << std::endl;
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

    if((unsigned int)(-1) == position_status || size == (unsigned int) -1 )
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

void clean(cl_context* contexts, cl_program* programs)
{
    if(programs != NULL && *programs != NULL){
        if( CL_SUCCESS != clReleaseProgram(*programs))
            std::cerr << "Unable to release program" << std::endl;
    }else
        std::cerr << "Unable to release program" << std::endl;


    if(contexts != NULL && *contexts != NULL){
        if(CL_SUCCESS != clReleaseContext(*contexts))
            std::cerr << "Unable to release context" << std::endl;
    }else
        std::cerr << "Unable to release context" << std::endl;
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

    int option = 0;
    bool print_device_info = 0;
    char* input_file = NULL;
    unsigned int num_input_files = 0;
    std::string compiler_options;

    while ((option = getopt(argc, argv, "DIsc:")) != -1)
        switch (option)
        {
            case 'I': // Include Path
                compiler_options += " -I .";
                //FIXME: optarg seems to be null
                //compiler_options += optarg;
                break;
            case 'D': // #define
                //compiler_options += " -D ";
                //FIXME: optarg seems to be null
                //compiler_options += optarg;
                break;
            case 'c': // input source file
                input_file = optarg;
                num_input_files++;
                break;
            case 's': //TODO honor this flag
                print_device_info = 1;
                break;
            case '?':
                if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr,
                            "Unknown option character `\\x%x'.\n",
                            optopt);
                return 1;
            default:
                return false;
        }
    // Stop Here if we have no input
    if(input_file == NULL)
    {
        std::cerr << "No input file specified by -c" << std::endl;
        return false;
    }

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
    const char* source = read_source(input_file);

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
        } else
        {
            std::cerr << "Encountered error with return code " << return_code  << std::endl;
            return false;
        }

    }
    if(CL_SUCCESS != clBuildProgram(target_program, num_devices, devices,/* OPTIONS */ compiler_options.c_str(), NULL, NULL))
    {
        //
#define ERROR_BUFFER_SIZE 4096 * 8
        char* buffer = new char[ERROR_BUFFER_SIZE];

        //FIXME Get the build log for all devices, not just the first
        int status = clGetProgramBuildInfo(target_program,devices[0], CL_PROGRAM_BUILD_STATUS,0,NULL,NULL);
        if(0 == status)
        {
            // We dont care
        }
        else if(status == CL_BUILD_NONE)
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

        status = clGetProgramBuildInfo(target_program, devices[0], CL_PROGRAM_BUILD_LOG, ERROR_BUFFER_SIZE, buffer, NULL);
        if(CL_SUCCESS == status)
            for(unsigned int i = 0; i < ERROR_BUFFER_SIZE && buffer[i]; ++i)
                std::cerr << buffer[i];
        else if(CL_INVALID_VALUE == status)
            std::cerr << "Invalid value when attempting to fetch build log, the error log\
                may have been bigger than the error buffer size of: " << ERROR_BUFFER_SIZE << "bytes" << std::endl;
        else
            std::cerr << "Unable to fetch build log" << status << std::endl;

        return false;
    }
    // TODO change this from argv1 to all of them
    std::cout << "compiled " << input_file << " successfully" << std::endl;
    clean(&compiler_context, &target_program);
    return 0;
}

