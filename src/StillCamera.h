#pragma once

#include "ofMain.h"




// Standard port setting for the camera component
#define MMAL_CAMERA_CAPTURE_PORT 2



// Stills format information
#define STILLS_FRAME_RATE_NUM 3
#define STILLS_FRAME_RATE_DEN 1

#define OUTPUT_BUFFERS_NUM 3

#define MAX_USER_EXIF_TAGS      32
#define MAX_EXIF_PAYLOAD_LENGTH 128

#include "CameraSettings.h"


class StillCamera
{
public:
	StillCamera();
	~StillCamera();
	void setup();
	string takePhoto();
	
    VCOS_SEMAPHORE_T complete_semaphore;	// semaphore which is posted when we reach end of frame (indicates end of capture or fault)
    MMAL_POOL_T*		pool;								// Pointer to the pool of buffers used by encoder output port
    MMAL_FOURCC_T		encoding;
    ofBuffer myBuffer;
    CameraSettings cameraSettings;
    bool doWriteFile;
    
    int					timeout;									// Time taken before frame is grabbed and app then shuts down. Units are milliseconds
    int					width;                          
    int					height;                         
    int					quality;									// JPEG quality setting (1-100)
    int					wantRAW;									// Flag for whether the JPEG metadata also contains the RAW bayer image

    const char*			exifTags[MAX_USER_EXIF_TAGS];				// Array of pointers to tags supplied from the command line
    int numExifTags;												// Number of supplied tags
    
    static void encoder_buffer_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);
    static void camera_control_callback(MMAL_PORT_T *port, MMAL_BUFFER_HEADER_T *buffer);

private:
	
	void create_camera_component();
	void create_encoder_component();
	MMAL_PORT_T* camera_still_port;
	MMAL_PORT_T* encoder_input_port;
	MMAL_PORT_T* encoder_output_port;
	MMAL_COMPONENT_T* camera;
	MMAL_COMPONENT_T* encoder;
    MMAL_CONNECTION_T*	encoder_connection;							// Pointer to the connection from camera to encoder
    
    void add_exif_tags();
    MMAL_STATUS_T add_exif_tag(const char* exif_tag);
    
    void close();    
    void onUpdateDuringExit(ofEventArgs& args);
	void addExitHandler();
    bool hasExitHandler;
    bool hasClosed;
    static void signal_handler(int signum);
    static bool doExit;
};