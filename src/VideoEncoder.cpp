//
//  VideoEncoder.cpp
//  OF_JVCRPI_WORKSPACE
//
//  Created by jason van cleave on 3/20/15.
//
//

#include "VideoEncoder.h"

VideoEncoder::VideoEncoder()
{
    pool = NULL;
    encoder = NULL;
    inputPort = NULL;
    outputPort = NULL;
    intraperiod = -1;    // Not set
    quantisationParameter = 0;

    immutableInput = 1;
    profile = MMAL_VIDEO_PROFILE_H264_HIGH;
    
    inlineMotionVectors = false;
    bitrate = 17000000; // This is a decent default bitrate for 1080p

    intra_refresh_type = MMAL_VIDEO_INTRA_REFRESH_DUMMY;
    videoFileName = ofGetTimestampString()+".h264";
    isRecording = false;
    doRecording = false;
}


MMAL_STATUS_T VideoEncoder::setup()
{
    MMAL_COMPONENT_T *encoder = 0;
    
    MMAL_STATUS_T status;
    
    status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_ENCODER, &encoder);
    
    MMAL_TRACE(status);

    
    inputPort = encoder->input[0];
    outputPort = encoder->output[0];
    
    // We want same format on input and output
    mmal_format_copy(outputPort->format, inputPort->format);
    
    // Only supporting H264 at the moment
    outputPort->format->encoding = MMAL_ENCODING_H264;
    
    outputPort->format->bitrate = bitrate;
    
    outputPort->buffer_size = outputPort->buffer_size_recommended;
    
    if (outputPort->buffer_size < outputPort->buffer_size_min)
    {
        outputPort->buffer_size = outputPort->buffer_size_min;
    }
    
    outputPort->buffer_num = outputPort->buffer_num_recommended;
    
    if (outputPort->buffer_num < outputPort->buffer_num_min)
    {
        outputPort->buffer_num = outputPort->buffer_num_min;
    }
    
    // We need to set the frame rate on output to 0, to ensure it gets
    // updated correctly from the input framerate when port connected
    outputPort->format->es->video.frame_rate.num = 0;
    outputPort->format->es->video.frame_rate.den = 1;
    
    // Commit the port changes to the output port
    status = mmal_port_format_commit(outputPort);
    MMAL_TRACE(status);

    
    // Set the rate control parameter
    if (0)
    {
        
        MMAL_PARAMETER_VIDEO_RATECONTROL_T param;
        param.hdr.id = MMAL_PARAMETER_RATECONTROL;
        param.hdr.size = sizeof(param);
        param.control = MMAL_VIDEO_RATECONTROL_DEFAULT;
        status = mmal_port_parameter_set(outputPort, &param.hdr);
        MMAL_TRACE(status);

        
    }
    
    if (intraperiod != -1)
    {
        MMAL_PARAMETER_UINT32_T param;
        param.hdr.id = MMAL_PARAMETER_INTRAPERIOD;
        param.hdr.size = sizeof(param);
        param.value = intraperiod;
        status = mmal_port_parameter_set(outputPort, &param.hdr);
        MMAL_TRACE(status);

    }
    
    if (quantisationParameter)
    {
        MMAL_PARAMETER_UINT32_T param;
        param.hdr.id = MMAL_PARAMETER_VIDEO_ENCODE_INITIAL_QUANT;
        param.hdr.size = sizeof(param);
        param.value = quantisationParameter;
        status = mmal_port_parameter_set(outputPort, &param.hdr);
        MMAL_TRACE(status);

        
        MMAL_PARAMETER_UINT32_T param2;
        param2.hdr.id = MMAL_PARAMETER_VIDEO_ENCODE_MIN_QUANT;
        param2.hdr.size = sizeof(param2);
        param2.value = quantisationParameter;
        status = mmal_port_parameter_set(outputPort, &param2.hdr);
        MMAL_TRACE(status);

        MMAL_PARAMETER_UINT32_T param3;
        param3.hdr.id = MMAL_PARAMETER_VIDEO_ENCODE_MAX_QUANT;
        param3.hdr.size = sizeof(param3);
        param3.value = quantisationParameter;
        status = mmal_port_parameter_set(outputPort, &param3.hdr);
        MMAL_TRACE(status);

        
    }
    
    {
        MMAL_PARAMETER_VIDEO_PROFILE_T  param;
        param.hdr.id = MMAL_PARAMETER_PROFILE;
        param.hdr.size = sizeof(param);
        
        param.profile[0].profile = profile;
        param.profile[0].level = MMAL_VIDEO_LEVEL_H264_4; // This is the only value supported
        
        status = mmal_port_parameter_set(outputPort, &param.hdr);
        MMAL_TRACE(status);

    }
    status = mmal_port_parameter_set_boolean(inputPort, 
                                             MMAL_PARAMETER_VIDEO_IMMUTABLE_INPUT, 
                                             immutableInput);
    MMAL_TRACE(status);
    
    
    //set INLINE HEADER flag to generate SPS and PPS for every IDR if requested
    status =  mmal_port_parameter_set_boolean(outputPort, 
                                              MMAL_PARAMETER_VIDEO_ENCODE_INLINE_HEADER, 
                                              inlineMotionVectors);
    MMAL_TRACE(status);

    //set INLINE VECTORS flag to request motion vector estimates
    status =  mmal_port_parameter_set_boolean(outputPort, 
                                              MMAL_PARAMETER_VIDEO_ENCODE_INLINE_VECTORS, 
                                              inlineMotionVectors);
    MMAL_TRACE(status);
    
    // Adaptive intra refresh settings
    if (intra_refresh_type != MMAL_VIDEO_INTRA_REFRESH_DUMMY)
    {
        MMAL_PARAMETER_VIDEO_INTRA_REFRESH_T  param;
        param.hdr.id = MMAL_PARAMETER_VIDEO_INTRA_REFRESH;
        param.hdr.size = sizeof(param);
        
        // Get first so we don't overwrite anything unexpectedly
        status = mmal_port_parameter_get(outputPort, &param.hdr);
        
        param.refresh_mode = intra_refresh_type;
        
        //if (intra_refresh_type == MMAL_VIDEO_INTRA_REFRESH_CYCLIC_MROWS)
        //   param.cir_mbs = 10;
        
        status = mmal_port_parameter_set(outputPort, &param.hdr);
         MMAL_TRACE(status);
    }
    
    //  Enable component
    status = mmal_component_enable(encoder);
    MMAL_TRACE(status);
    
    /* Create pool of buffer headers for the output port to consume */
    pool = mmal_port_pool_create(outputPort, outputPort->buffer_num, outputPort->buffer_size);
    
    if (!pool)
    {
        vcos_log_error("Failed to create buffer header pool for encoder output port %s", outputPort->name);
    }
    
    
    return status;


}
/**
 *  buffer header callback function for encoder
 *
 *  Callback will dump buffer data to the specific file
 *
 * @param port Pointer to port from which callback originated
 * @param buffer mmal buffer header pointer
 */
void VideoEncoder::encoder_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
    VideoEncoder* self = (VideoEncoder *)port->userdata;
    if(self && self->doRecording)
    {
        mmal_buffer_header_mem_lock(buffer);
        // We are pushing data into a circular buffer
        //memcpy(pData->cb_buff + pData->cb_wptr, buffer->data, copy_to_end);
        //memcpy(pData->cb_buff, buffer->data + copy_to_end, copy_to_start);
        
        self->fileBuffer.append((const char*)buffer->data, buffer->length);
        
        mmal_buffer_header_mem_unlock(buffer);
        ofLogVerbose() << self->fileBuffer.size();
        
    }
    mmal_buffer_header_release(buffer);
    // and send one back to the port (if still open)
    if (port->is_enabled)
    {
        MMAL_STATUS_T status;
        
        MMAL_BUFFER_HEADER_T *new_buffer = mmal_queue_get(self->pool->queue);
        
        if (new_buffer)
        {
            status = mmal_port_send_buffer(port, new_buffer);
            //MMAL_TRACE(status);
        }
        
        if (!new_buffer || status != MMAL_SUCCESS)
        {
            ofLogError() << "Unable to return a buffer to the encoder port";
        }
     
    }
}

MMAL_STATUS_T VideoEncoder::enableOutputPort()
{
    //TODO move to setup?
    outputPort->userdata = (struct MMAL_PORT_USERDATA_T *)this;
    MMAL_STATUS_T status = mmal_port_enable(outputPort, &VideoEncoder::encoder_buffer_callback);
    MMAL_TRACE(status);
    // Send all the buffers to the encoder output port
    int num = mmal_queue_length(pool->queue);
    int q;
    for (q=0;q<num;q++)
    {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(pool->queue);
        
        if (!buffer)
        {
            ofLog(OF_LOG_ERROR, "Unable to get a required buffer %d from pool queue", q);
        }
        MMAL_STATUS_T status = mmal_port_send_buffer(outputPort, buffer);
        MMAL_TRACE(status);
        if(status != MMAL_SUCCESS)
        {
            ofLog(OF_LOG_ERROR, "Unable to send a buffer to encoder output port (%d)", q);
        }
    }
    return status;
}
void VideoEncoder::startRecording()
{
    doRecording = true;
    if(!isRecording)
    {
        stringstream fileName;
        fileName << ofGetTimestampString();
        fileName << "_";
        fileName << "bitrate_" << ofToString(bitrate);
        videoFileName = fileName.str();
        
        isRecording = true;
    }
}
bool VideoEncoder::stopRecording()
{
    doRecording = false;
    isRecording = false;
    string filePath = ofToDataPath(videoFileName+".h264", true);
    bool didWrite = ofBufferToFile(filePath, fileBuffer);
    if(didWrite)
    {
        ofLogVerbose(__func__) << "FILE SUCCESS filePath: " << filePath;
    }else
    {
        ofLogError(__func__) << "FILE FAIL filePath: " << filePath; 
    }
    return didWrite;
}

VideoEncoder::~VideoEncoder()
{
    
    if(encoder)
    {
        MMAL_STATUS_T status = mmal_component_destroy(encoder);
        MMAL_TRACE(status, "Destroy encoder");
    }
    
    
}

MMAL_PORT_T* VideoEncoder::getInputPort()
{
    return inputPort;
}