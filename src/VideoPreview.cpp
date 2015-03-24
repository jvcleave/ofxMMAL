//
//  VideoPreview.cpp
//  OF_JVCRPI_WORKSPACE
//
//  Created by jason van cleave on 3/20/15.
//
//

#include "VideoPreview.h"

VideoPreview::VideoPreview()
{
    wantPreview = true;
    wantFullScreenPreview = 1;
    opacity = 255;
    previewWindow.x = 0;
    previewWindow.y = 0;
    previewWindow.width = 1024;
    previewWindow.height = 768;
    preview_component = NULL;
    preview_port = NULL;
}

MMAL_STATUS_T VideoPreview::setup()
{
    
    
    MMAL_STATUS_T status;
    
    if (!wantPreview)
    {
        // No preview required, so create a null sink component to take its place
        status = mmal_component_create("vc.null_sink", &preview_component);
        MMAL_TRACE(status);
    }
    else
    {
        status = mmal_component_create(MMAL_COMPONENT_DEFAULT_VIDEO_RENDERER,
                                       &preview_component);
        
        MMAL_TRACE(status);
        preview_port = preview_component->input[0];
        
        MMAL_DISPLAYREGION_T param;
        param.hdr.id = MMAL_PARAMETER_DISPLAYREGION;
        param.hdr.size = sizeof(MMAL_DISPLAYREGION_T);
        
        param.set = MMAL_DISPLAY_SET_LAYER;
        param.layer = PREVIEW_LAYER;
        
        param.set |= MMAL_DISPLAY_SET_ALPHA;
        param.alpha = opacity;
        
        if (wantFullScreenPreview)
        {
            param.set |= MMAL_DISPLAY_SET_FULLSCREEN;
            param.fullscreen = 1;
        }
        else
        {
            param.set |= (MMAL_DISPLAY_SET_DEST_RECT | MMAL_DISPLAY_SET_FULLSCREEN);
            param.fullscreen = 0;
            param.dest_rect = previewWindow;
        }
        
        status = mmal_port_parameter_set(preview_port, &param.hdr);
        MMAL_TRACE(status);
    }
    
    /* Enable component */
    status = mmal_component_enable(preview_component);
    MMAL_TRACE(status);
    
    
    return status;


}

MMAL_PORT_T* VideoPreview::getInputPort()
{
    return preview_component->input[0];
}