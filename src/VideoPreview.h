#pragma once

#include "ofMain.h"


#include "CameraSettings.h"

#define PREVIEW_LAYER      2


class VideoPreview
{
public:
    VideoPreview();
    MMAL_STATUS_T setup();
    bool wantPreview;                       /// Display a preview
    int wantFullScreenPreview;             /// 0 is use previewRect, non-zero to use full screen
    int opacity;                           /// Opacity of window - 0 = transparent, 255 = opaque
    MMAL_RECT_T previewWindow;             /// Destination rectangle for the preview window.
    MMAL_COMPONENT_T* preview_component;   /// Pointer to the created preview display component
    MMAL_PORT_T* preview_port;
    MMAL_PORT_T* getInputPort();
};