#pragma once

#include "interface/mmal/mmal.h"
#include "interface/mmal/mmal_logging.h"
#include "interface/mmal/mmal_buffer.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"


extern inline
string mmalStatusToString(MMAL_STATUS_T status)
{
    string output = "unknown";
    switch (status) 
    {
            
        case MMAL_SUCCESS: {output="Success"; } break; 
        case MMAL_ENOMEM: {output="Out of memory"; } break; 
        case MMAL_ENOSPC: {output="Out of resources (other than memory)"; } break; 
        case MMAL_EINVAL: {output="Argument is invalid"; } break; 
        case MMAL_ENOSYS: {output="Function not implemented"; } break; 
        case MMAL_ENOENT: {output="No such file or directory"; } break; 
        case MMAL_ENXIO: {output="No such device or address"; } break; 
        case MMAL_EIO: {output="IO error"; } break; 
        case MMAL_ESPIPE: {output="Illegal seek"; } break; 
        case MMAL_ECORRUPT: {output="Data is corrupt attention FIXME: not POSIX"; } break; 
        case MMAL_ENOTREADY: {output="Component is not ready attention FIXME: not POSIX"; } break; 
        case MMAL_ECONFIG: {output="Component is not configured attention FIXME: not POSIX"; } break; 
        case MMAL_EISCONN: {output="Port is already connected"; } break; 
        case MMAL_ENOTCONN: {output="Port is disconnected"; } break; 
        case MMAL_EAGAIN: {output="Resource temporarily unavailable. Try again later"; } break; 
        case MMAL_EFAULT: {output="Bad address"; } break; 
        case MMAL_STATUS_MAX: {output="whatever"; } break;
            
    }
    return output;
}

extern inline
string vcosStatusToString(VCOS_STATUS_T status)
{
    string output = "unknown";
    switch (status) 
    {
            
        case VCOS_SUCCESS :{output = "VCOS_SUCCESS"; } break;
        case VCOS_EAGAIN :{output = "VCOS_EAGAIN"; } break;
        case VCOS_ENOENT :{output = "VCOS_ENOENT"; } break;
        case VCOS_ENOSPC :{output = "VCOS_ENOSPC"; } break;
        case VCOS_EINVAL :{output = "VCOS_EINVAL"; } break;
        case VCOS_EACCESS :{output = "VCOS_EACCESS"; } break;
        case VCOS_ENOMEM :{output = "VCOS_ENOMEM"; } break;
        case VCOS_EEXIST :{output = "VCOS_EEXIST"; } break;
        case VCOS_ENXIO :{output = "VCOS_ENXIO"; } break;
        case VCOS_EINTR :{output = "VCOS_EINTR"; } break; 
        case VCOS_ENOSYS :{output = "VCOS_ENOSYS"; } break; 
            
    }
    return output;
}



extern inline
void logMMALStatus(MMAL_STATUS_T status, string comments="", string functionName="", int lineNumber=0)
{
    string commentLine = " ";
    if(!comments.empty())
    {
        commentLine = " " + comments + " ";
    }
    
    ofLogVerbose(functionName) << lineNumber << commentLine << mmalStatusToString(status);
}

#define ENABLE_MMAL_TRACE 1
#define __func__ __PRETTY_FUNCTION__

#define MMAL_TRACE_1_ARGS(status)                      logMMALStatus(status, "", __func__, __LINE__);
#define MMAL_TRACE_2_ARGS(status, comments)            logMMALStatus(status, comments, __func__, __LINE__);
#define MMAL_TRACE_3_ARGS(status, comments, whatever)  logMMALStatus(status, comments, __func__, __LINE__);

#define GET_MMAL_TRACE_4TH_ARG(arg1, arg2, arg3, arg4, ...) arg4
#define MMAL_TRACE_MACRO_CHOOSER(...) GET_MMAL_TRACE_4TH_ARG(__VA_ARGS__, MMAL_TRACE_3_ARGS, MMAL_TRACE_2_ARGS, MMAL_TRACE_1_ARGS, )

#if defined (ENABLE_MMAL_TRACE)
    //#warning enabling MMAL_TRACE
#define MMAL_TRACE(...) MMAL_TRACE_MACRO_CHOOSER(__VA_ARGS__)(__VA_ARGS__)
#else
    #warning  disabling MMAL_TRACE
    #warning  disabling -Wunused-but-set-variable -Wunused-variable
    #define MMAL_TRACE(...)
    #pragma GCC diagnostic ignored "-Wunused-but-set-variable"
    #pragma GCC diagnostic ignored "-Wunused-variable"
#endif

/**
 * Connect two specific ports together
 *
 * @param output_port Pointer the output port
 * @param input_port Pointer the input port
 * @param Pointer to a mmal connection pointer, reassigned if function successful
 * @return Returns a MMAL_STATUS_T giving result of operation
 *
 */
extern inline
MMAL_STATUS_T connect_ports(MMAL_PORT_T *output_port, 
                                   MMAL_PORT_T *input_port, 
                                   MMAL_CONNECTION_T **connection)
{
    MMAL_STATUS_T status;
    
    status =  mmal_connection_create(connection, 
                                     output_port, 
                                     input_port, 
                                     MMAL_CONNECTION_FLAG_TUNNELLING | MMAL_CONNECTION_FLAG_ALLOCATION_ON_INPUT);
    
    if (status == MMAL_SUCCESS)
    {
        status =  mmal_connection_enable(*connection);
        if (status != MMAL_SUCCESS)
        {
            mmal_connection_destroy(*connection);
        }
    }
    
    return status;
}

