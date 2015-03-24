#pragma once

#include "ofMain.h"


#include "CameraSettings.h"
#include "VideoPreview.h"
#include "VideoEncoder.h"

#define MMAL_CAMERA_PREVIEW_PORT 0
#define MMAL_CAMERA_VIDEO_PORT 1
#define MMAL_CAMERA_CAPTURE_PORT 2
// Frames rates of 0 implies variable, but denominator needs to be 1 to prevent div by 0
#define PREVIEW_FRAME_RATE_NUM 0
#define PREVIEW_FRAME_RATE_DEN 1
#define VIDEO_FRAME_RATE_DEN 1
#define VIDEO_OUTPUT_BUFFERS_NUM 3

class VideoCamera
{
public:
    VideoCamera();
    ~VideoCamera();
    void close();
    void setup();
    CameraSettings cameraSettings;
    MMAL_STATUS_T create_camera_component();
    
    static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
    
    
    int timeout;                        /// Time taken before frame is grabbed and app then shuts down. Units are milliseconds
    int width;                          /// Requested width of image
    int height;                         /// requested height of image
    int framerate;                      /// Requested frame rate (fps)

    
    //RASPIPREVIEW_PARAMETERS preview_parameters;   /// Preview setup parameters
    //RASPICAM_CAMERA_PARAMETERS camera_parameters; /// Camera setup parameters
    
    MMAL_COMPONENT_T *camera;    /// Pointer to the camera component
    VideoEncoder videoEncoder;
    VideoPreview videoPreview;
    MMAL_CONNECTION_T *preview_connection; /// Pointer to the connection from camera to preview
    MMAL_CONNECTION_T *encoder_connection; /// Pointer to the connection from camera to encoder
    
    MMAL_POOL_T *encoder_pool; /// Pointer to the pool of buffers used by encoder output port
    
    MMAL_PORT_T* preview_port;
    MMAL_PORT_T* video_port;
    MMAL_PORT_T* still_port;
    MMAL_PORT_T* encoder_port;
    //PORT_USERDATA callback_data;        /// Used to move data to the encoder callback
    

    
    int cameraNum;                       /// Camera number
    int settings;                        /// Request settings from the camera
    int sensor_mode;			            /// Sensor mode. 0=auto. Check docs/forum for modes selected by other values.
    bool wantPreview;
private:
    bool hasClosed;
};