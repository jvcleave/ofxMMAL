#pragma once

#include "ofMain.h"
#include "MMAL_Utils.h"



struct MMAL_PARAM_COLOURFX_T
{
	int enable;       // Turn colourFX on or off
	int u,v;          // U and V to use
};

class CameraSettings
{
public:
	CameraSettings();


	int rotation;              // 0-359
	bool hflip;                 // 0 or 1
	bool vflip;                 // 0 or 1
	
	MMAL_COMPONENT_T* camera;
    void setup(MMAL_COMPONENT_T* camera_, bool doApplyDefaultSettings = true);
    void applyDefaultSettings();
    
	MMAL_STATUS_T	set_saturation(int);
	MMAL_STATUS_T	set_sharpness(int);
	MMAL_STATUS_T	set_contrast(int);
	MMAL_STATUS_T	set_brightness(int);
	MMAL_STATUS_T	set_ISO(int);
	MMAL_STATUS_T	set_metering_mode(MMAL_PARAM_EXPOSUREMETERINGMODE_T);
	MMAL_STATUS_T	set_video_stabilisation(bool);
	MMAL_STATUS_T	set_exposure_compensation(int);
	MMAL_STATUS_T	set_exposure_mode(MMAL_PARAM_EXPOSUREMODE_T);
	MMAL_STATUS_T	set_awb_mode(MMAL_PARAM_AWBMODE_T);
	MMAL_STATUS_T	set_imageFX(MMAL_PARAM_IMAGEFX_T);
	MMAL_STATUS_T	set_colourFX(const MMAL_PARAM_COLOURFX_T *);
	void            set_rotation(int);
	void            set_flips(bool hflip, bool vflip);
	
	int	get_saturation(){return saturation;}
    int	get_sharpness(){return sharpness;}
    int	get_contrast(){return contrast;}
    int	get_brightness(){return brightness;}
    int	get_ISO(){return ISO;}
    MMAL_PARAM_EXPOSUREMETERINGMODE_T	get_metering_mode(){return exposureMeterMode;}
    bool get_video_stabilisation(){return videoStabilisation;}
    int get_exposure_compensation(){return exposureCompensation;}
	//MMAL_PARAM_THUMBNAIL_CONFIG_T		get_thumbnail_parameters();
    MMAL_PARAM_EXPOSUREMODE_T   get_exposure_mode(){return exposureMode;}
	MMAL_PARAM_AWBMODE_T        get_awb_mode(){return awbMode;}
	MMAL_PARAM_IMAGEFX_T        get_imageFX(){return imageEffect;}
	MMAL_PARAM_COLOURFX_T       get_colourFX(){return colourEffects;}
private:
    int sharpness;             // -100 to 100
    int contrast;              // -100 to 100
    int brightness;            //  0 to 100
    int saturation;            //  -100 to 100
    int ISO;                   //  TODO : what range?
    bool videoStabilisation;    // 0 or 1 (false or true)
    int exposureCompensation;  // -10 to +10 ?
    
    
    MMAL_PARAM_EXPOSUREMODE_T			exposureMode;
    MMAL_PARAM_EXPOSUREMETERINGMODE_T	exposureMeterMode;
    MMAL_PARAM_AWBMODE_T				awbMode;
    MMAL_PARAM_IMAGEFX_T				imageEffect;
    MMAL_PARAMETER_IMAGEFX_PARAMETERS_T imageEffectsParameters;
    MMAL_PARAM_COLOURFX_T				colourEffects;
    
};
