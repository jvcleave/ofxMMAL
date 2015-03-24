#pragma once

#include "ofMain.h"


#include "CameraSettings.h"

struct PORT_USERDATA
{
    FILE *file_handle;                   /// File handle to write buffer data to.
    //RASPIVID_STATE *pstate;              /// pointer to our state in case required in callback
    int abort;                           /// Set to 1 in callback if an error occurs to attempt to abort the capture
    char *cb_buff;                       /// Circular buffer
    int   cb_len;                        /// Length of buffer
    int   cb_wptr;                       /// Current write pointer
    int   cb_wrap;                       /// Has buffer wrapped at least once?
    int   cb_data;                       /// Valid bytes in buffer
#define IFRAME_BUFSIZE (60*1000)
    int   iframe_buff[IFRAME_BUFSIZE];          /// buffer of iframe pointers
    int   iframe_buff_wpos;
    int   iframe_buff_rpos;
    char  header_bytes[29];
    int  header_wptr;
    FILE *imv_file_handle;               /// File handle to write inline motion vectors to.
};

class VideoEncoder
{
    public:
    VideoEncoder();
    ~VideoEncoder();
    MMAL_STATUS_T enableOutputPort();
    MMAL_STATUS_T setup();
    void enableCircularBuffer();
    int bitrate;                        /// Requested bitrate

    MMAL_POOL_T *pool; /// Pointer to the pool of buffers used by encoder output port
    MMAL_COMPONENT_T *encoder;   /// Pointer to the encoder component
    int intraperiod;                    /// Intra-refresh period (key frame rate)
    int quantisationParameter;          /// Quantisation parameter - quality. Set bitrate 0 and set this for variable bitrate
    int bInlineHeaders;                  /// Insert inline headers to stream (SPS, PPS)
    char *filename;                     /// filename of output file
    int verbose;                        /// !0 if want detailed run information
    int demoMode;                       /// Run app in demo mode
    int demoInterval;                   /// Interval between camera settings changes
    int immutableInput;                 /// Flag to specify whether encoder works in place or creates a new buffer. Result is preview can display either
    /// the camera output or the encoder output (with compression artifacts)
    MMAL_VIDEO_PROFILE_T profile;                        /// H264 profile to use for encoding
    int waitMethod;                     /// Method for switching between pause and capture
    
    int onTime;                         /// In timed cycle mode, the amount of time the capture is on per cycle
    int offTime;                        /// In timed cycle mode, the amount of time the capture is off per cycle
    
    int segmentSize;                    /// Segment mode In timed cycle mode, the amount of time the capture is off per cycle
    int segmentWrap;                    /// Point at which to wrap segment counter
    int segmentNumber;                  /// Current segment counter
    int splitNow;                       /// Split at next possible i-frame if set to 1.
    int splitWait;                      /// Switch if user wants splited files 

    int bCapturing;                     /// State of capture/pause
    int bCircularBuffer;                /// Whether we are writing to a circular buffer
    
    int inlineMotionVectors;             /// Encoder outputs inline Motion Vectors
    char *imv_filename;                  /// filename of inline Motion Vectors output
    
    MMAL_VIDEO_INTRA_REFRESH_T intra_refresh_type;              /// What intra refresh type to use. -1 to not set.
    
    MMAL_PORT_T* getInputPort();
    MMAL_PORT_T* outputPort;
    MMAL_PORT_T* inputPort;
    
    static void encoder_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

};