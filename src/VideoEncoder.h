#pragma once

#include "ofMain.h"


#include "CameraSettings.h"

class VideoEncoder
{
    public:
    VideoEncoder();
    bool isRecording;
    bool doRecording;
    bool stopRecording();
    void startRecording();
    string videoFileName;
    ~VideoEncoder();
    MMAL_STATUS_T enableOutputPort();
    MMAL_STATUS_T setup();
    void enableCircularBuffer();
    int bitrate;                        // Requested bitrate

    MMAL_POOL_T *pool;                  // Pointer to the pool of buffers used by encoder output port
    MMAL_COMPONENT_T *encoder;          // Pointer to the encoder component
    int intraperiod;                    // Intra-refresh period (key frame rate)
    int quantisationParameter;          // Quantisation parameter - quality. Set bitrate 0 and set this for variable bitrate
    bool inlineMotionVectors;                  // Insert inline headers to stream (SPS, PPS)
    int immutableInput;                 // Flag to specify whether encoder works in place or creates a new buffer. Result is preview can display either
    // the camera output or the encoder output (with compression artifacts)
    MMAL_VIDEO_PROFILE_T profile;                        // H264 profile to use for encoding
    

    
    MMAL_VIDEO_INTRA_REFRESH_T intra_refresh_type;              // What intra refresh type to use. -1 to not set.
    
    MMAL_PORT_T* getInputPort();
    MMAL_PORT_T* outputPort;
    MMAL_PORT_T* inputPort;
    ofBuffer fileBuffer;
    static void encoder_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

};