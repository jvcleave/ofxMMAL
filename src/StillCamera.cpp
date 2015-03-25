/*
 *  StillCamera.cpp
 *  openFrameworksLib
 *
 *  Created by jason van cleave on 5/16/13.
 *  Copyright 2013 jasonvancleave.com. All rights reserved.
 *
 */

#include "StillCamera.h"

StillCamera::StillCamera()
{
	camera_still_port = NULL;
	encoder_input_port = NULL;
	encoder_output_port = NULL;
	camera = NULL;
	encoder = NULL;
    hasExitHandler = false;
    doWriteFile = false;
    encoding = MMAL_ENCODING_JPEG;
    timeout = 0; // 5s delay before take image
    //width = 2592;
   // height = 1944;
    width = 1280;
    height = 720;
    quality = 100;
    wantRAW = 0;
    //filename = NULL;
    camera = NULL;
    numExifTags = 0;
    hasClosed = false;
}

void StillCamera::setup()
{
    addExitHandler();
	MMAL_STATUS_T status = MMAL_SUCCESS;
	

	create_camera_component();
	create_encoder_component();
	
	camera_still_port   = camera->output[MMAL_CAMERA_CAPTURE_PORT];
	encoder_input_port  = encoder->input[0];
	encoder_output_port = encoder->output[0];
    status = mmal_port_parameter_set_boolean(camera_still_port, MMAL_PARAMETER_ZERO_COPY, MMAL_TRUE);
    MMAL_TRACE(status);
    status = mmal_port_parameter_set_boolean(encoder_input_port, MMAL_PARAMETER_ZERO_COPY, MMAL_TRUE);
    MMAL_TRACE(status);
    //status = mmal_port_parameter_set_boolean(encoder_output_port, MMAL_PARAMETER_ZERO_COPY, MMAL_TRUE);
    //MMAL_TRACE(status);

	VCOS_STATUS_T vcos_status;
	
	ofLogVerbose() << "Connecting camera stills port to encoder input port";
		
	
	// Now connect the camera to the encoder
	status = connect_ports(camera_still_port, encoder_input_port, &encoder_connection);
    MMAL_TRACE(status, "connect_ports");
	

    vcos_status = vcos_semaphore_create(&complete_semaphore, "RaspiStill-sem", 0);
	
	encoder_output_port->userdata = (struct MMAL_PORT_USERDATA_T *)this;
	
	
	// Enable the encoder output port and tell it its callback function
	status = mmal_port_enable(encoder_output_port, &StillCamera::encoder_buffer_callback);
	MMAL_TRACE(status, "enable encoder_output_port");


	
}

string StillCamera::takePhoto()
{
    cameraSettings.setup(camera);
    MMAL_STATUS_T status;
	
    if(timeout)
    {
       vcos_sleep(timeout); 
    }
	
	// Open the file
	string fileName = ofToDataPath(ofGetTimestampString()+".jpg", true);
	
	add_exif_tags();
	
    int num = mmal_queue_length(pool->queue);
    ofLogVerbose() << "mmal_queue_length" << num;
    
    for (int q=0; q<num; q++)
    {
        MMAL_BUFFER_HEADER_T *buffer = mmal_queue_get(pool->queue);
        
        if (!buffer)
        {
            ofLogVerbose() << "Unable to get a required buffer " << q << " from pool queue";
        }
        status = mmal_port_send_buffer(encoder_output_port, buffer);
        MMAL_TRACE(status, "mmal_port_send_buffer encoder_output_port");
    }
    
    status = mmal_port_parameter_set_boolean(camera_still_port, MMAL_PARAMETER_CAPTURE, 1);
    MMAL_TRACE(status, "Starting capture");
    if (status == MMAL_SUCCESS)
    {
        // Wait for capture to complete
        // For some reason using vcos_semaphore_wait_timeout sometimes returns immediately with bad parameter error
        // even though it appears to be all correct, so reverting to untimed one until figure out why its erratic
        vcos_semaphore_wait(&complete_semaphore);
        if(doWriteFile)
        {
            ofBufferToFile(fileName, myBuffer); 
            ofLogVerbose() << "Finished capture " << fileName;
        }
        myBuffer.clear();
    }
    

	
	vcos_semaphore_delete(&complete_semaphore);
    return fileName;
	
}


void StillCamera::create_camera_component()
{
	
	MMAL_ES_FORMAT_T *format;
	MMAL_STATUS_T status;
	
	/* Create the component */
	status = mmal_component_create(MMAL_COMPONENT_DEFAULT_CAMERA, &camera);
    MMAL_TRACE(status, "mmal_component_create MMAL_COMPONENT_DEFAULT_CAMERA");

	camera_still_port = camera->output[MMAL_CAMERA_CAPTURE_PORT];
	
	// Enable the camera, and tell it its control callback function
	status = mmal_port_enable(camera->control, &StillCamera::camera_control_callback);
    MMAL_TRACE(status, "set Camera callback");

	
	//raspicamcontrol_set_all_parameters(camera, &photo.camera_parameters);
	
    format = camera_still_port->format;
	
	// Set our stills format on the stills (for encoder) port
	format->encoding = MMAL_ENCODING_OPAQUE;
    format->encoding_variant = MMAL_ENCODING_I420;

    
	format->es->video.width = VCOS_ALIGN_UP(width, 32);
	format->es->video.height = VCOS_ALIGN_UP(height, 16);
	format->es->video.crop.x = 0;
	format->es->video.crop.y = 0;
	format->es->video.crop.width = width;
	format->es->video.crop.height = height;
	format->es->video.frame_rate.num = STILLS_FRAME_RATE_NUM;
	format->es->video.frame_rate.den = STILLS_FRAME_RATE_DEN;
	
	status = mmal_port_format_commit(camera_still_port);
    MMAL_TRACE(status, "camera still format set");

	
	/* Ensure there are enough buffers to avoid dropping frames */
	if (camera_still_port->buffer_num < OUTPUT_BUFFERS_NUM)
	{
		camera_still_port->buffer_num = OUTPUT_BUFFERS_NUM;
	}
	
	/* Enable component */
	status = mmal_component_enable(camera);
    MMAL_TRACE(status, "Enable Camera");	
	if (wantRAW)
	{
        status = mmal_port_parameter_set_boolean(camera_still_port, MMAL_PARAMETER_ENABLE_RAW_CAPTURE, 1);
		MMAL_TRACE(status, "Enable RAW");
	}
	
	
	ofLogVerbose() << "Camera component done";
}



void StillCamera::create_encoder_component()
{
	MMAL_STATUS_T status;
	
	status = mmal_component_create(MMAL_COMPONENT_DEFAULT_IMAGE_ENCODER, &encoder);
	MMAL_TRACE(status, "Create encoder");

	encoder_input_port = encoder->input[0];
	encoder_output_port = encoder->output[0];
	
	// We want same format on input and output
	mmal_format_copy(encoder_output_port->format, encoder_input_port->format);
    
	// Specify out output format
	encoder_output_port->format->encoding = encoding;
	
	encoder_output_port->buffer_size = encoder_output_port->buffer_size_recommended;
	
	if (encoder_output_port->buffer_size < encoder_output_port->buffer_size_min)
	{
		encoder_output_port->buffer_size = encoder_output_port->buffer_size_min;
	}
	
	encoder_output_port->buffer_num = encoder_output_port->buffer_num_recommended;
	
	if (encoder_output_port->buffer_num < encoder_output_port->buffer_num_min)
	{
		encoder_output_port->buffer_num = encoder_output_port->buffer_num_min;
	}
	
	status = mmal_port_format_commit(encoder_output_port);
    MMAL_TRACE(status, "Commit the port changes to encoder_output_port");

	

	status = mmal_port_parameter_set_uint32(encoder_output_port, MMAL_PARAMETER_JPEG_Q_FACTOR, quality);
    MMAL_TRACE(status, "Set the JPEG quality level");


	//  Enable component
	status = mmal_component_enable(encoder);
	MMAL_TRACE(status, "Enable encoder");
	
	/* Create pool of buffer headers for the output port to consume */
	pool = mmal_port_pool_create(encoder_output_port, encoder_output_port->buffer_num, encoder_output_port->buffer_size);
	
	if (!pool)
	{
		ofLogVerbose() << "Failed to create buffer header pool for encoder output port " << encoder_output_port->name;
	}else 
	{
		ofLogVerbose() << "pool creation PASS";
	}
}


/**
 * Add an exif tag to the capture
 *
 * @param state Pointer to state control struct
 * @param exif_tag String containing a "key=value" pair.
 * @return  Returns a MMAL_STATUS_T giving result of operation
 */
MMAL_STATUS_T StillCamera::add_exif_tag(const char *exif_tag)
{
    MMAL_STATUS_T status;
    MMAL_PARAMETER_EXIF_T *exif_param = (MMAL_PARAMETER_EXIF_T*)calloc(sizeof(MMAL_PARAMETER_EXIF_T) + MAX_EXIF_PAYLOAD_LENGTH, 1);
     
     //vcos_assert(this);
     //vcos_assert(encoder_component);
     
     // Check to see if the tag is present or is indeed a key=value pair.
    if (!exif_tag || strchr(exif_tag, '=') == NULL || strlen(exif_tag) > MAX_EXIF_PAYLOAD_LENGTH-1)
    {
        return MMAL_EINVAL;
    }
     
    exif_param->hdr.id = MMAL_PARAMETER_EXIF;
     
    strncpy((char*)exif_param->data, exif_tag, MAX_EXIF_PAYLOAD_LENGTH-1);
     
    exif_param->hdr.size = sizeof(MMAL_PARAMETER_EXIF_T) + strlen((char*)exif_param->data);
     
    status = mmal_port_parameter_set(encoder->output[0], &exif_param->hdr);
    MMAL_TRACE(status);
    free(exif_param);

    return status;
}

/**
 * Add a basic set of EXIF tags to the capture
 * Make, Time etc
 *
 * @param state Pointer to state control struct
 *
 */
void StillCamera::add_exif_tags()
{
    time_t rawtime;
    struct tm *timeinfo;
    char time_buf[32];
    char exif_buf[128];
    int i;
    
    add_exif_tag("IFD0.Model=RP_OV5647");
    add_exif_tag("IFD0.Make=RaspberryPi");
    
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    
    snprintf(time_buf, sizeof(time_buf),
             "%04d:%02d:%02d:%02d:%02d:%02d",
             timeinfo->tm_year+1900,
             timeinfo->tm_mon+1,
             timeinfo->tm_mday,
             timeinfo->tm_hour,
             timeinfo->tm_min,
             timeinfo->tm_sec);
    
    snprintf(exif_buf, sizeof(exif_buf), "EXIF.DateTimeDigitized=%s", time_buf);
    add_exif_tag(exif_buf);
    
    snprintf(exif_buf, sizeof(exif_buf), "EXIF.DateTimeOriginal=%s", time_buf);
    add_exif_tag(exif_buf);
    
    snprintf(exif_buf, sizeof(exif_buf), "IFD0.DateTime=%s", time_buf);
    add_exif_tag(exif_buf);
    
    // Now send any user supplied tags
    
    for (i=0;i<numExifTags && i < MAX_USER_EXIF_TAGS; i++)
    {
        if (exifTags[i])
        {
            add_exif_tag(exifTags[i]);
        }
    }
}

/**
 *  buffer header callback function for camera control
 *
 *  No actions taken in current version
 *
 * @param port Pointer to port from which callback originated
 * @param buffer mmal buffer header pointer
 */
void StillCamera::camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
    if (buffer->cmd == MMAL_EVENT_PARAMETER_CHANGED)
    {
    }
    else
    {
        ofLog(OF_LOG_VERBOSE, "Received unexpected camera control callback event, 0x%08x", buffer->cmd);
        
    }
    
    mmal_buffer_header_release(buffer);
}



/**
 *  buffer header callback function for encoder
 *
 *  Callback will dump buffer data to the specific file
 *
 * @param port Pointer to port from which callback originated
 * @param buffer mmal buffer header pointer
 */
void StillCamera::encoder_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer)
{
    bool complete = false;
    bool success = false;
    // We pass our file handle and other stuff in via the userdata field.
    
    StillCamera* grabber = (StillCamera *)port->userdata;
    MMAL_STATUS_T status;
    
    
    if (grabber)
    {
        if (buffer->length)
        {
            ofLogVerbose() << "buffer->length:  " << buffer->length;
            status = mmal_buffer_header_mem_lock(buffer);
            MMAL_TRACE(status);
            grabber->myBuffer.append((const char*)buffer->data, buffer->length);            
            mmal_buffer_header_mem_unlock(buffer);
        }
        
        if (buffer->flags & (MMAL_BUFFER_HEADER_FLAG_FRAME_END | MMAL_BUFFER_HEADER_FLAG_TRANSMISSION_FAILED))
        {
            complete = true;
            if(buffer->flags & MMAL_BUFFER_HEADER_FLAG_FRAME_END)
            {
                success = true;
            }
            if(buffer->flags & MMAL_BUFFER_HEADER_FLAG_TRANSMISSION_FAILED)
            {
                success = false;
            }
        }
        
    }
    else
    {
        ofLogVerbose("Received a encoder buffer callback with no state");
    }
    
    // release buffer back to the pool
    mmal_buffer_header_release(buffer);
    
    // and send one back to the port (if still open)
    if (port->is_enabled)
    {
        MMAL_STATUS_T status;
        MMAL_BUFFER_HEADER_T *new_buffer;
        
        new_buffer = mmal_queue_get(grabber->pool->queue);
        
        if (new_buffer)
        {
            status = mmal_port_send_buffer(port, new_buffer);
        }
        if (!new_buffer || status != MMAL_SUCCESS)
            ofLogVerbose("Unable to return a buffer to the encoder port");
    }
    
    if (complete)
    {
        if(success)
        {
            grabber->doWriteFile = true;
        }
        vcos_semaphore_post(&(grabber->complete_semaphore));
        
    }
    
    
}





bool StillCamera::doExit = false;

void StillCamera::signal_handler(int signum)
{
    cout << "StillCamera caught signal " << signum;
    StillCamera::doExit = true;
}

inline
void StillCamera::onUpdateDuringExit(ofEventArgs& args)
{
    if (StillCamera::doExit)
    {
        ofLogVerbose(__func__) << " EXITING VIA SIGNAL";
        close();
        ofExit();
    }
}

inline
void StillCamera::addExitHandler()
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
        new_action.sa_handler = StillCamera::signal_handler;
        
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
    
    ofAddListener(ofEvents().update, this, &StillCamera::onUpdateDuringExit);
    hasExitHandler = true;
}



void StillCamera::close()
{
    MMAL_STATUS_T status;
    if (camera)
    {
        status = mmal_component_destroy(camera);
        MMAL_TRACE(status, "Destroy camera");
        camera = NULL;
    }
    if (encoder)
    {
        status = mmal_component_destroy(encoder);
        MMAL_TRACE(status, "Destroy encoder");
        encoder = NULL;
    }
    hasClosed = true;
}

StillCamera::~StillCamera()
{
    if(!hasClosed)
    {
        close();
    }
    
}

