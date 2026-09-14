#pragma once

#include "SWRender.h"

#include <libavcodec/avcodec.h>

#ifdef __cplusplus
extern "C"
{
#endif

    extern int SWRender_LoadImageToBuffer(const char *filepath, SWRenderBuffer *buffer);
    extern int SWRender_SaveImageFromBuffer(SWRenderBuffer *buffer, const char *filepath, enum AVCodecID codec_id);

#ifdef __cplusplus
}
#endif
