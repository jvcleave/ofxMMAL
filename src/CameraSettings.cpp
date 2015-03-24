/*
 *  CameraSettings.cpp
 *  openFrameworksLib
 *
 *  Created by jason van cleave on 5/17/13.
 *  Copyright 2013 jasonvancleave.com. All rights reserved.
 *
 */

#include "CameraSettings.h"

CameraSettings::CameraSettings()
{
	sharpness = 0;
	contrast = 0;
	brightness = 50;
	saturation = 0;
	ISO = 400;
	videoStabilisation = false;
	exposureCompensation = 0;
	exposureMode = MMAL_PARAM_EXPOSUREMODE_AUTO;
	exposureMeterMode = MMAL_PARAM_EXPOSUREMETERINGMODE_AVERAGE;
	awbMode = MMAL_PARAM_AWBMODE_AUTO;
	imageEffect = MMAL_PARAM_IMAGEFX_NONE;
	colourEffects.enable = 0;
	colourEffects.u = 128;
	colourEffects.v = 128;
	rotation = 0;
	hflip = true;
	vflip = false;
	
	camera = NULL;
}


void CameraSettings::setup(MMAL_COMPONENT_T *camera_, bool doApplyDefaultSettings)
{
	camera = camera_;
    if(doApplyDefaultSettings)
    {
        applyDefaultSettings();
    }

}
void CameraSettings::applyDefaultSettings()
{
    set_saturation(saturation);
    set_sharpness(sharpness);
    set_contrast(contrast);
    set_brightness(brightness);
    //set_ISO(ISO); TODO Not working for some reason
    set_video_stabilisation(videoStabilisation);
    set_exposure_compensation(exposureCompensation);
    set_exposure_mode(exposureMode);
    set_metering_mode(exposureMeterMode);
    set_awb_mode(awbMode);
    set_imageFX(imageEffect);
    set_colourFX(&colourEffects);
    //set_thumbnail_parameters(camera, &params->thumbnailConfig);  TODO Not working for some reason
    set_rotation(rotation);
    set_flips(hflip, vflip);
}


/**
 * Adjust the saturation level for images
 * @param camera Pointer to camera component
 * @param saturation Value to adjust, -100 to 100
 * @return 0 if successful, non-zero if any parameters out of range
 */
MMAL_STATUS_T CameraSettings::set_saturation(int saturation)
{
	
    MMAL_STATUS_T status = MMAL_ENOTREADY;
	
	if (saturation >= -100 && saturation <= 100)
	{
		MMAL_RATIONAL_T value = {saturation, 100};
		status = mmal_port_parameter_set_rational(camera->control, MMAL_PARAMETER_SATURATION, value);
	}
	else
	{
		ofLog(OF_LOG_ERROR, "Invalid saturation value");
		
	}
    MMAL_TRACE(status);
    return status;
	
}

/**
 * Set the sharpness of the image
 * @param camera Pointer to camera component
 * @param sharpness Sharpness adjustment -100 to 100
 */
MMAL_STATUS_T CameraSettings::set_sharpness(int sharpness_)
{
	
	MMAL_STATUS_T status = MMAL_ENOTREADY; if(!camera) return status;	
	if (sharpness_ >= -100 && sharpness_ <= 100)
	{
		MMAL_RATIONAL_T value = {sharpness_, 100};
        sharpness = sharpness_;
		status = mmal_port_parameter_set_rational(camera->control, MMAL_PARAMETER_SHARPNESS, value);
	}
	else
	{
		ofLog(OF_LOG_ERROR, "Invalid sharpness value");
		
	}
	MMAL_TRACE(status);
    return status;
}

/**
 * Set the contrast adjustment for the image
 * @param camera Pointer to camera component
 * @param contrast Contrast adjustment -100 to  100
 * @return
 */
MMAL_STATUS_T CameraSettings::set_contrast(int contrast)
{
	
	MMAL_STATUS_T status = MMAL_ENOTREADY; if(!camera) return status;

	if (contrast >= -100 && contrast <= 100)
	{
		MMAL_RATIONAL_T value = {contrast, 100};
		status = mmal_port_parameter_set_rational(camera->control, MMAL_PARAMETER_CONTRAST, value);
	}
	else
	{
		ofLog(OF_LOG_ERROR, "Invalid contrast value");
		
	}
    MMAL_TRACE(status);
	return status;
	
}

/**
 * Adjust the brightness level for images
 * @param camera Pointer to camera component
 * @param brightness Value to adjust, 0 to 100
 * @return 0 if successful, non-zero if any parameters out of range
 */
MMAL_STATUS_T CameraSettings::set_brightness(int brightness)
{
	MMAL_STATUS_T status = MMAL_ENOTREADY; if(!camera) return status;
	if (brightness >= 0 && brightness <= 100)
	{
		MMAL_RATIONAL_T value = {brightness, 100};
		status = mmal_port_parameter_set_rational(camera->control, MMAL_PARAMETER_BRIGHTNESS, value);
	}
	else
	{
		ofLog(OF_LOG_ERROR, "Invalid brightness value");
		
	}
    MMAL_TRACE(status);
    return status;
	
}

/**
 * Adjust the ISO used for images
 * @param camera Pointer to camera component
 * @param ISO Value to set TODO :
 * @return 0 if successful, non-zero if any parameters out of range
 */
MMAL_STATUS_T CameraSettings::set_ISO(int ISO)
{
   	MMAL_STATUS_T status = MMAL_ENOTREADY; if(!camera) return status;

	status = mmal_port_parameter_set_uint32(camera->control, MMAL_PARAMETER_ISO, ISO);
    MMAL_TRACE(status);
    return status;
}

/**
 * Adjust the metering mode for images
 * @param camera Pointer to camera component
 * @param saturation Value from following
 *   - MMAL_PARAM_EXPOSUREMETERINGMODE_AVERAGE,
 *   - MMAL_PARAM_EXPOSUREMETERINGMODE_SPOT,
 *   - MMAL_PARAM_EXPOSUREMETERINGMODE_BACKLIT,
 *   - MMAL_PARAM_EXPOSUREMETERINGMODE_MATRIX
 * @return 0 if successful, non-zero if any parameters out of range
 */

MMAL_STATUS_T CameraSettings::set_metering_mode(MMAL_PARAM_EXPOSUREMETERINGMODE_T m_mode )
{
    MMAL_STATUS_T status = MMAL_ENOTREADY; if(!camera) return status;

	MMAL_PARAMETER_EXPOSUREMETERINGMODE_T meter_mode = {{MMAL_PARAMETER_EXP_METERING_MODE,sizeof(meter_mode)},
		m_mode};
	 status = mmal_port_parameter_set(camera->control, &meter_mode.hdr);
    MMAL_TRACE(status);
    return status;
}


/**
 * Set the video stabilisation flag. Only used in video mode
 * @param camera Pointer to camera component
 * @param saturation Flag 0 off 1 on
 * @return 0 if successful, non-zero if any parameters out of range
 */
MMAL_STATUS_T CameraSettings::set_video_stabilisation(bool vstabilisation)
{
    MMAL_STATUS_T status = MMAL_ENOTREADY; if(!camera) return status;

    status = mmal_port_parameter_set_boolean(camera->control, 
                                             MMAL_PARAMETER_VIDEO_STABILISATION, 
                                             vstabilisation);
    MMAL_TRACE(status);
    return status;
}

/**
 * Adjust the exposure compensation for images (EV)
 * @param camera Pointer to camera component
 * @param exp_comp Value to adjust, -10 to +10
 * @return 0 if successful, non-zero if any parameters out of range
 */
MMAL_STATUS_T CameraSettings::set_exposure_compensation(int exp_comp)
{
    MMAL_STATUS_T status = MMAL_ENOTREADY; if(!camera) return status;

	status = mmal_port_parameter_set_int32(camera->control, 
                                           MMAL_PARAMETER_EXPOSURE_COMP , 
                                           exp_comp);
    MMAL_TRACE(status);
    return status;
}


/**
 * Set exposure mode for images
 * @param camera Pointer to camera component
 * @param mode Exposure mode to set from
 *   - MMAL_PARAM_EXPOSUREMODE_OFF,
 *   - MMAL_PARAM_EXPOSUREMODE_AUTO,
 *   - MMAL_PARAM_EXPOSUREMODE_NIGHT,
 *   - MMAL_PARAM_EXPOSUREMODE_NIGHTPREVIEW,
 *   - MMAL_PARAM_EXPOSUREMODE_BACKLIGHT,
 *   - MMAL_PARAM_EXPOSUREMODE_SPOTLIGHT,
 *   - MMAL_PARAM_EXPOSUREMODE_SPORTS,
 *   - MMAL_PARAM_EXPOSUREMODE_SNOW,
 *   - MMAL_PARAM_EXPOSUREMODE_BEACH,
 *   - MMAL_PARAM_EXPOSUREMODE_VERYLONG,
 *   - MMAL_PARAM_EXPOSUREMODE_FIXEDFPS,
 *   - MMAL_PARAM_EXPOSUREMODE_ANTISHAKE,
 *   - MMAL_PARAM_EXPOSUREMODE_FIREWORKS,
 *
 * @return 0 if successful, non-zero if any parameters out of range
 */
MMAL_STATUS_T CameraSettings::set_exposure_mode(MMAL_PARAM_EXPOSUREMODE_T mode)
{
    MMAL_STATUS_T status = MMAL_ENOTREADY; if(!camera) return status;

    MMAL_PARAMETER_EXPOSUREMODE_T exp_mode = {{MMAL_PARAMETER_EXPOSURE_MODE,sizeof(exp_mode)}, mode};
	status = mmal_port_parameter_set(camera->control, &exp_mode.hdr);
    MMAL_TRACE(status);
    return status;
}


/**
 * Set the aWB (auto white balance) mode for images
 * @param camera Pointer to camera component
 * @param awb_mode Value to set from
 *   - MMAL_PARAM_AWBMODE_OFF,
 *   - MMAL_PARAM_AWBMODE_AUTO,
 *   - MMAL_PARAM_AWBMODE_SUNLIGHT,
 *   - MMAL_PARAM_AWBMODE_CLOUDY,
 *   - MMAL_PARAM_AWBMODE_SHADE,
 *   - MMAL_PARAM_AWBMODE_TUNGSTEN,
 *   - MMAL_PARAM_AWBMODE_FLUORESCENT,
 *   - MMAL_PARAM_AWBMODE_INCANDESCENT,
 *   - MMAL_PARAM_AWBMODE_FLASH,
 *   - MMAL_PARAM_AWBMODE_HORIZON,
 * @return 0 if successful, non-zero if any parameters out of range
 */
MMAL_STATUS_T CameraSettings::set_awb_mode(MMAL_PARAM_AWBMODE_T awb_mode)
{
	
    MMAL_STATUS_T status = MMAL_ENOTREADY; if(!camera) return status;

    MMAL_PARAMETER_AWBMODE_T param = {{MMAL_PARAMETER_AWB_MODE,sizeof(param)}, awb_mode};
    status = mmal_port_parameter_set(camera->control, &param.hdr);
    MMAL_TRACE(status);
    return status;
}

/**
 * Set the image effect for the images
 * @param camera Pointer to camera component
 * @param imageFX Value from
 *   - MMAL_PARAM_IMAGEFX_NONE,
 *   - MMAL_PARAM_IMAGEFX_NEGATIVE,
 *   - MMAL_PARAM_IMAGEFX_SOLARIZE,
 *   - MMAL_PARAM_IMAGEFX_POSTERIZE,
 *   - MMAL_PARAM_IMAGEFX_WHITEBOARD,
 *   - MMAL_PARAM_IMAGEFX_BLACKBOARD,
 *   - MMAL_PARAM_IMAGEFX_SKETCH,
 *   - MMAL_PARAM_IMAGEFX_DENOISE,
 *   - MMAL_PARAM_IMAGEFX_EMBOSS,
 *   - MMAL_PARAM_IMAGEFX_OILPAINT,
 *   - MMAL_PARAM_IMAGEFX_HATCH,
 *   - MMAL_PARAM_IMAGEFX_GPEN,
 *   - MMAL_PARAM_IMAGEFX_PASTEL,
 *   - MMAL_PARAM_IMAGEFX_WATERCOLOUR,
 *   - MMAL_PARAM_IMAGEFX_FILM,
 *   - MMAL_PARAM_IMAGEFX_BLUR,
 *   - MMAL_PARAM_IMAGEFX_SATURATION,
 *   - MMAL_PARAM_IMAGEFX_COLOURSWAP,
 *   - MMAL_PARAM_IMAGEFX_WASHEDOUT,
 *   - MMAL_PARAM_IMAGEFX_POSTERISE,
 *   - MMAL_PARAM_IMAGEFX_COLOURPOINT,
 *   - MMAL_PARAM_IMAGEFX_COLOURBALANCE,
 *   - MMAL_PARAM_IMAGEFX_CARTOON,
 * @return 0 if successful, non-zero if any parameters out of range
 */
MMAL_STATUS_T CameraSettings::set_imageFX(MMAL_PARAM_IMAGEFX_T imageFX)
{
    MMAL_STATUS_T status = MMAL_ENOTREADY; if(!camera) return status;

    MMAL_PARAMETER_IMAGEFX_T imgFX = {{MMAL_PARAMETER_IMAGE_EFFECT,sizeof(imgFX)}, imageFX};
	status = mmal_port_parameter_set(camera->control, &imgFX.hdr);
    MMAL_TRACE(status);
    return status;
    
}

/* TODO :what to do with the image effects parameters?
 MMAL_PARAMETER_IMAGEFX_PARAMETERS_T imfx_param = {{MMAL_PARAMETER_IMAGE_EFFECT_PARAMETERS,sizeof(imfx_param)},
 imageFX, 0, {0}};
 mmal_port_parameter_set(camera->control, &imfx_param.hdr);
 */

/**
 * Set the colour effect  for images (Set UV component)
 * @param camera Pointer to camera component
 * @param colourFX  Contains enable state and U and V numbers to set (e.g. 128,128 = Black and white)
 * @return 0 if successful, non-zero if any parameters out of range
 */
MMAL_STATUS_T CameraSettings::set_colourFX(const MMAL_PARAM_COLOURFX_T *colourFX)
{
	
    MMAL_STATUS_T status = MMAL_ENOTREADY; if(!camera) return status;

    MMAL_PARAMETER_COLOURFX_T colfx = {{MMAL_PARAMETER_COLOUR_EFFECT,sizeof(colfx)}, 0, 0, 0};
	colfx.enable = colourFX->enable;
	colfx.u = colourFX->u;
	colfx.v = colourFX->v;
	 
	status = mmal_port_parameter_set(camera->control, &colfx.hdr);
    MMAL_TRACE(status);
    return status;
	
}


/**
 * Set the rotation of the image
 * @param camera Pointer to camera component
 * @param rotation Degree of rotation (any number, but will be converted to 0,90,180 or 270 only)
 * @return 0 if successful, non-zero if any parameters out of range
 */
void CameraSettings::set_rotation(int rotation)
{
    if(!camera) return;

	int my_rotation = ((rotation % 360 ) / 90) * 90;
    MMAL_STATUS_T status;
	status = mmal_port_parameter_set_int32(camera->output[0], MMAL_PARAMETER_ROTATION, my_rotation);MMAL_TRACE(status);
	status = mmal_port_parameter_set_int32(camera->output[1], MMAL_PARAMETER_ROTATION, my_rotation);MMAL_TRACE(status);
	status = mmal_port_parameter_set_int32(camera->output[2], MMAL_PARAMETER_ROTATION, my_rotation);MMAL_TRACE(status);
	
	
}

/**
 * Set the flips state of the image
 * @param camera Pointer to camera component
 * @param hflip If true, horizontally flip the image
 * @param vflip If true, vertically flip the image
 *
 * @return 0 if successful, non-zero if any parameters out of range
 */
void CameraSettings::set_flips(bool hflip, bool vflip)
{
    if(!camera) return;
	MMAL_PARAMETER_MIRROR_T mirror = {{MMAL_PARAMETER_MIRROR, sizeof(MMAL_PARAMETER_MIRROR_T)}, MMAL_PARAM_MIRROR_NONE};
	
	if (hflip && vflip)
		mirror.value = MMAL_PARAM_MIRROR_BOTH;
	else
		if (hflip)
			mirror.value = MMAL_PARAM_MIRROR_HORIZONTAL;
		else
			if (vflip)
				mirror.value = MMAL_PARAM_MIRROR_VERTICAL;
    MMAL_STATUS_T status;

	status = mmal_port_parameter_set(camera->output[0], &mirror.hdr);MMAL_TRACE(status);
	status = mmal_port_parameter_set(camera->output[1], &mirror.hdr);MMAL_TRACE(status);
	status = mmal_port_parameter_set(camera->output[2], &mirror.hdr);MMAL_TRACE(status);
}
