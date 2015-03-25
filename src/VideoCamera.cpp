#include "VideoCamera.h"


VideoCamera::VideoCamera()
{
    timeout = 5000;     // 5s delay before take image
    width = 1920;       // Default to 1080p
    height = 1080;
    framerate = 30;

    cameraNum = 0;
    settings = 0;
    sensor_mode = 0;
    
    
    // Setup preview window defaults
    //raspipreview_set_defaults(&preview_parameters);
    
    // Set up the camera_parameters to default
    //raspicamcontrol_set_defaults(&camera_parameters);
    
    camera = NULL;
    video_port = NULL;
    still_port = NULL;
    preview_port = NULL;
    wantPreview = true;
    hasClosed = false;
    hasExitHandler = false;

}

void VideoCamera::setup()
{
    addExitHandler();
    create_camera_component();
    MMAL_STATUS_T status = videoPreview.setup();
    MMAL_TRACE(status);
    status = videoEncoder.setup();
    MMAL_TRACE(status);
    //cameraSettings.setup(camera);
    if(wantPreview)
    {
        // Connect camera to preview
        status = connect_ports(preview_port, 
                               videoPreview.getInputPort(), 
                               &preview_connection);
        MMAL_TRACE(status);

    }
    // Now connect the camera to the encoder
    status = connect_ports(video_port, 
                           videoEncoder.getInputPort(), 
                           &encoder_connection);
    MMAL_TRACE(status);
    
    status =  mmal_port_parameter_set_boolean(video_port, MMAL_PARAMETER_CAPTURE, true);
    MMAL_TRACE(status);
    
    status = videoEncoder.enableOutputPort();
    MMAL_TRACE(status);

}

void VideoCamera::startRecording()
{
    videoEncoder.startRecording();
}
void VideoCamera::stopRecording()
{
    videoEncoder.stopRecording();

}

/**
 *  buffer header callback function for camera control
 *
 *  Callback will dump buffer data to the specific file
 *
 * @param port Pointer to port from which callback originated
 * @param buffer mmal buffer header pointer
 */
void VideoCamera::camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
    if (buffer->cmd == MMAL_EVENT_PARAMETER_CHANGED)
    {
        MMAL_EVENT_PARAMETER_CHANGED_T *param = (MMAL_EVENT_PARAMETER_CHANGED_T *)buffer->data;
        switch (param->hdr.id) {
            case MMAL_PARAMETER_CAMERA_SETTINGS:
            {
                MMAL_PARAMETER_CAMERA_SETTINGS_T *settings = (MMAL_PARAMETER_CAMERA_SETTINGS_T*)param;
                ofLog(OF_LOG_VERBOSE, "Exposure now %u, analog gain %u/%u, digital gain %u/%u",
                               settings->exposure,
                               settings->analog_gain.num, settings->analog_gain.den,
                               settings->digital_gain.num, settings->digital_gain.den);
                ofLog(OF_LOG_VERBOSE, "AWB R=%u/%u, B=%u/%u",
                               settings->awb_red_gain.num, settings->awb_red_gain.den,
                               settings->awb_blue_gain.num, settings->awb_blue_gain.den
                               );
            }
                break;
        }
    }
    else
    {
        vcos_log_error("Received unexpected camera control callback event, 0x%08x", buffer->cmd);
    }
    
    mmal_buffer_header_release(buffer);
}

/**
 * Create the camera component, set up its ports
 *
 * @param state Pointer to state control struct
 *
 * @return MMAL_SUCCESS if all OK, something else otherwise
 *
 */

MMAL_STATUS_T VideoCamera::create_camera_component()
{
    MMAL_ES_FORMAT_T *format;
    
    MMAL_STATUS_T status;
    
    /* Create the component */
    status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);
    MMAL_TRACE(status);
    
    MMAL_PARAMETER_INT32_T camera_num;
    camera_num.hdr.id=MMAL_PARAMETER_CAMERA_NUM;
    camera_num.hdr.size = sizeof(camera_num);
    camera_num.value = cameraNum;
    status = mmal_port_parameter_set(camera->control, &camera_num.hdr);
    MMAL_TRACE(status);
    
   
    
    status = mmal_port_parameter_set_uint32(camera->control, MMAL_PARAMETER_CAMERA_CUSTOM_SENSOR_CONFIG, sensor_mode);
    
    MMAL_TRACE(status);
    
    preview_port = camera->output[MMAL_CAMERA_PREVIEW_PORT];
    video_port = camera->output[MMAL_CAMERA_VIDEO_PORT];
    still_port = camera->output[MMAL_CAMERA_CAPTURE_PORT];
    
    if (settings)
    {
        MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T change_event_request;
        change_event_request.hdr.id = MMAL_PARAMETER_CHANGE_EVENT_REQUEST;
        change_event_request.hdr.size = sizeof(MMAL_PARAMETER_CHANGE_EVENT_REQUEST_T);
        change_event_request.change_id = MMAL_PARAMETER_CAMERA_SETTINGS;
        change_event_request.enable = true;
        
        status = mmal_port_parameter_set(camera->control, &change_event_request.hdr);
        MMAL_TRACE(status);
    }
    
    // Enable the camera, and tell it its control callback function
    status = mmal_port_enable(camera->control, &VideoCamera::camera_control_callback);
    MMAL_TRACE(status);
        
    //  set up the camera configuration
    {
        MMAL_PARAMETER_CAMERA_CONFIG_T cam_config;
        cam_config.hdr.id = MMAL_PARAMETER_CAMERA_CONFIG;
        cam_config.hdr.size = sizeof(cam_config);
        cam_config.max_stills_w = width;
        cam_config.max_stills_h = height;
        cam_config.stills_yuv422 = 0;
        cam_config.one_shot_stills = 0;
        cam_config.max_preview_video_w = width;
        cam_config.max_preview_video_h = height;
        cam_config.num_preview_video_frames = 3;
        cam_config.stills_capture_circular_buffer_height = 0;
        cam_config.fast_preview_resume = 0,
        cam_config.use_stc_timestamp = MMAL_PARAM_TIMESTAMP_MODE_RESET_STC;
        mmal_port_parameter_set(camera->control, &cam_config.hdr);
    }
    
    // Now set up the port formats
    
    // Set the encode format on the Preview port
    // HW limitations mean we need the preview to be the same size as the required recorded output
    
    format = preview_port->format;
    
    format->encoding = MMAL_ENCODING_OPAQUE;
    format->encoding_variant = MMAL_ENCODING_I420;
    int shutter_speed = 0; //TODO 
    if(shutter_speed > 6000000)
    {
        MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
            { 50, 1000 }, {166, 1000}};
            mmal_port_parameter_set(preview_port, &fps_range.hdr);
    }else
    {
        if(shutter_speed > 1000000)
        {
            MMAL_PARAMETER_FPS_RANGE_T fps_range = {{MMAL_PARAMETER_FPS_RANGE, sizeof(fps_range)},
                { 166, 1000 }, {999, 1000}};
            mmal_port_parameter_set(preview_port, &fps_range.hdr);
        }
        
    }
    
    //enable dynamic framerate if necessary
    if (shutter_speed)
    {   
        if (framerate > 1000000./shutter_speed)
        {
            framerate=0;
            ofLogVerbose() << "Enable dynamic frame rate to fulfil shutter speed requirement";
        }
    } 
    
    format->encoding = MMAL_ENCODING_OPAQUE;
    format->es->video.width = VCOS_ALIGN_UP(width, 32);
    format->es->video.height = VCOS_ALIGN_UP(height, 16);
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = width;
    format->es->video.crop.height = height;
    format->es->video.frame_rate.num = PREVIEW_FRAME_RATE_NUM;
    format->es->video.frame_rate.den = PREVIEW_FRAME_RATE_DEN;
    
    status = mmal_port_format_commit(preview_port);
    
    MMAL_TRACE(status);
    
    // Set the encode format on the video  port
    
    format = video_port->format;
    format->encoding_variant = MMAL_ENCODING_I420;
    
    if(shutter_speed > 6000000)
    {
        MMAL_PARAMETER_FPS_RANGE_T fps_range;
        fps_range.hdr.id = MMAL_PARAMETER_FPS_RANGE;
        fps_range.hdr.size = sizeof(fps_range);
        fps_range.fps_low.num = 50;
        fps_range.fps_low.den = 1000;
        fps_range.fps_high.num = 166;
        fps_range.fps_high.den = 1000;
        mmal_port_parameter_set(video_port, &fps_range.hdr);
    }
    else if(shutter_speed > 1000000)
    {
        MMAL_PARAMETER_FPS_RANGE_T fps_range;
        fps_range.hdr.id = MMAL_PARAMETER_FPS_RANGE;
        fps_range.hdr.size = sizeof(fps_range);
        fps_range.fps_low.num = 167;
        fps_range.fps_low.den = 1000;
        fps_range.fps_high.num = 999;
        fps_range.fps_high.den = 1000;
        mmal_port_parameter_set(video_port, &fps_range.hdr);
    }
    
    format->encoding = MMAL_ENCODING_OPAQUE;
    format->es->video.width = VCOS_ALIGN_UP(width, 32);
    format->es->video.height = VCOS_ALIGN_UP(height, 16);
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = width;
    format->es->video.crop.height = height;
    format->es->video.frame_rate.num = framerate;
    format->es->video.frame_rate.den = VIDEO_FRAME_RATE_DEN;
    
    status = mmal_port_format_commit(video_port);
    
    MMAL_TRACE(status);
    
    // Ensure there are enough buffers to avoid dropping frames
    if (video_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
    {
        video_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;
    }
    
    
    // Set the encode format on the still  port
    
    format = still_port->format;
    
    format->encoding = MMAL_ENCODING_OPAQUE;
    format->encoding_variant = MMAL_ENCODING_I420;
    
    format->es->video.width = VCOS_ALIGN_UP(width, 32);
    format->es->video.height = VCOS_ALIGN_UP(height, 16);
    format->es->video.crop.x = 0;
    format->es->video.crop.y = 0;
    format->es->video.crop.width = width;
    format->es->video.crop.height = height;
    format->es->video.frame_rate.num = 0;
    format->es->video.frame_rate.den = 1;
    
    status = mmal_port_format_commit(still_port);
    
    MMAL_TRACE(status);

    
    /* Ensure there are enough buffers to avoid dropping frames */
    if (still_port->buffer_num < VIDEO_OUTPUT_BUFFERS_NUM)
    {
        still_port->buffer_num = VIDEO_OUTPUT_BUFFERS_NUM;
    }
    
    /* Enable component */
    status = mmal_component_enable(camera);
     MMAL_TRACE(status);
        
    cameraSettings.setup(camera);
    
    return status;

}
VideoCamera::~VideoCamera()
{
    if(!hasClosed)
    {
        close();
    }
}   

void VideoCamera::close()
{
    MMAL_STATUS_T status;
    if (camera)
    {
        status = mmal_component_destroy(camera);
        MMAL_TRACE(status, "Destroy camera");
        camera = NULL;
    }
    hasClosed = true;
}

bool VideoCamera::doExit = false;

void VideoCamera::signal_handler(int signum)
{
    cout << "VideoCamera caught signal " << signum;
    VideoCamera::doExit = true;
}

inline
void VideoCamera::onUpdateDuringExit(ofEventArgs& args)
{
    if (VideoCamera::doExit)
    {
        ofLogVerbose(__func__) << " EXITING VIA SIGNAL";
        close();
        ofExit();
    }
}

inline
void VideoCamera::addExitHandler()
{
    if(hasExitHandler) return;
    
    vector<int> signals;
    signals.push_back(SIGINT);
    signals.push_back(SIGQUIT);
    
    for (size_t i=0; i<signals.size(); i++)
    {
        int SIGNAL_TO_BLOCK = signals[i];
        //http://stackoverflow.com/questions/11465148/using-sigaction-c-cpp
        
        //Struct for the new action associated to the SIGNAL_TO_BLOCK
        struct sigaction new_action;
        new_action.sa_handler = VideoCamera::signal_handler;
        
        //Empty the sa_mask. This means that no signal is blocked while the signal_handler runs.
        sigemptyset(&new_action.sa_mask);
        
        //Block the SEGTERM signal so while the signal_handler runs, the SIGTERM signal is ignored
        sigaddset(&new_action.sa_mask, SIGTERM);
        
        //Remove any flag from sa_flag. See documentation for flags allowed
        new_action.sa_flags = 0;
        
        struct sigaction old_action;
        //Read the old signal associated to SIGNAL_TO_BLOCK
        sigaction(SIGNAL_TO_BLOCK, NULL, &old_action);
        
        //If the old handler wasn't SIG_IGN it is a handler that just "ignores" the signal
        if (old_action.sa_handler != SIG_IGN)
        {
            //Replace the signal handler of SIGNAL_TO_BLOCK with the one described by new_action
            sigaction(SIGNAL_TO_BLOCK, &new_action, NULL);
        }
        
    }
    
    ofAddListener(ofEvents().update, this, &VideoCamera::onUpdateDuringExit);
    hasExitHandler = true;
}

