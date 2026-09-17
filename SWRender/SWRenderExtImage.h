/*
    Copyright (C) 2026 Glacc

    This file is part of sw-render.

    sw-render is free software: you can redistribute it
    and/or modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation, either
    version 3 of the License, or (at your option) any later version.

    sw-render is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with sw-render. If not,
    see <https://www.gnu.org/licenses/>. 
*/

#pragma once

#include "SWRender.h"

#include <libavcodec/avcodec.h>

#define SWRENDER_IMG_AVIO_BUF_SIZE 4096

#ifdef __cplusplus
extern "C"
{
#endif

    extern int SWRender_LoadImageFromFileToBuffer(const char *filepath, SWRenderBuffer *buffer);
    extern int SWRender_LoadImageFromMemoryToBuffer(const uint8_t *src, const size_t size, SWRenderBuffer *buffer);
    extern int SWRender_SaveImageFromBuffer(SWRenderBuffer *buffer, const char *filepath, enum AVCodecID codec_id);

#ifdef __cplusplus
}
#endif
