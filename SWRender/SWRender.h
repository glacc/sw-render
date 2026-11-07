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

#include <stdint.h>
#include <float.h>

#define SWRENDER_ALIGNMENT 4

#define SWRENDER_ANTIALIAS_MULT 8
#define SWRENDER_ANTIALIAS_DIST 0.8F

#define SWRENDER_ANTIALIAS_DIV_MUL (1.0F / (SWRENDER_ANTIALIAS_MULT * SWRENDER_ANTIALIAS_MULT))

#define SWRENDER_LINE_BLOCK_SIZE_MIN 8
#define SWRENDER_LINE_BLOCK_SIZE_MAX 512

#define SWRENDER_LINE_SAFEZONE_SIZE 0.5F

#define SWRENDER_EPSILON FLT_EPSILON

#ifndef _SWRENDER_USE_NON_RGBA_PIXFMT_
typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
}
__attribute__((packed)) SWRenderColor;
#else
#include "SWRenderColor.h"
#endif

typedef struct
{
    int x, y, w, h;
}
SWRenderRectI;

typedef struct
{
    float x, y, w, h;
}
SWRenderRectF;

typedef struct
{
    int x1, y1, x2, y2;
}
SWRenderBoundI;

typedef struct
{
    float x1, y1, x2, y2;
}
SWRenderBoundF;

typedef struct
{
    int x, y;
}
SWRenderVec2I;

typedef struct
{
    float x, y;
}
SWRenderVec2F;

typedef struct
{
    int w;
    int h;
    uint32_t *data;
    int linesize;
}
SWRenderBuffer;

#ifdef __cplusplus
extern "C"
{
#endif

    // pointer
    extern SWRenderColor *SWRender_GetPointerByPosition(const SWRenderBuffer *buffer, int x, int y);
    extern SWRenderColor *SWRender_GetPointerNextLine(const SWRenderBuffer *buffer, SWRenderColor *ptr);

    // bounds
    extern void SWRender_CheckAndSwapBoundCornersInt(SWRenderBoundI *bounds);
    extern void SWRender_CheckAndSwapBoundCornersFloat(SWRenderBoundF *bounds);
    
    extern bool SWRender_GetIntersectionBetweenBoundInt(const SWRenderBoundI *bound_1, const SWRenderBoundI *bound_2, SWRenderBoundI *out_intersection);
    extern bool SWRender_GetIntersectionBetweenBoundFloat(const SWRenderBoundF *bound_1, const SWRenderBoundF *bound_2, SWRenderBoundF *out_intersection);
    extern void SWRender_IncludeBoundToBoundInt(SWRenderBoundI *dst, const SWRenderBoundI *bounds);
    extern void SWRender_IncludeBoundToBoundFloat(SWRenderBoundF *dst, const SWRenderBoundF *bounds);

    // rect <-> bounds
    extern bool SWRender_RectToBoundInt(SWRenderBoundI *bounds, const SWRenderRectI *rect);
    extern bool SWRender_RectToBoundFloat(SWRenderBoundF *bounds, const SWRenderRectF *rect);
    extern bool SWRender_BoundToRectInt(SWRenderRectI *rect, const SWRenderBoundI *bounds);
    extern bool SWRender_BoundToRectFloat(SWRenderRectF *rect, const SWRenderBoundF *bounds);

    // vector
    extern float SWRender_Vec2FLen(SWRenderVec2F vec);
    extern float SWRender_DistPtLineSeg(SWRenderVec2F point, const SWRenderBoundF *line);

    // vector between rect/bounds
    extern bool SWRender_IsVec2IInsideBoundI(const SWRenderVec2I vector, const SWRenderBoundI *bounds);
    extern bool SWRender_IsVec2FInsideBoundF(const SWRenderVec2F vector, const SWRenderBoundF *bounds);
    extern bool SWRender_IsVec2IInsideRectI(const SWRenderVec2I vector, const SWRenderRectI *rect);
    extern bool SWRender_IsVec2FInsideRectF(const SWRenderVec2F vector, const SWRenderRectF *rect);

    // buffer
    extern bool SWRender_CheckBuffer(const SWRenderBuffer *buffer);

    extern bool SWRender_BufferAlloc(SWRenderBuffer *buffer);
    extern void SWRender_BufferFree(SWRenderBuffer *buffer);

    extern bool SWRender_BufferGetRect(SWRenderRectI *rect, const SWRenderBuffer *buffer);
    extern bool SWRender_BufferGetBounds(SWRenderBoundI *bounds, const SWRenderBuffer *buffer);
    extern bool SWRender_BufferCrop(SWRenderBuffer *dst, const SWRenderBuffer *src, const SWRenderBoundI *bounds);

    // pixel
    extern void SWRender_AlphaBlendRGBA8888(SWRenderColor src, SWRenderColor *dst);

    // pixel (not implemented yet)
    extern void SWRender_SetPixel(SWRenderBuffer *buffer, SWRenderVec2I pos, SWRenderColor color);
    extern void SWRender_BlendPixel(SWRenderBuffer *buffer, SWRenderVec2I pos, SWRenderColor color);

    // shapes
    extern void SWRender_FillRect(SWRenderBuffer *buffer, const SWRenderBoundI *bounds, SWRenderColor color, bool overwrite);
    extern void SWRender_FillCircle(SWRenderBuffer *buffer, SWRenderVec2F center, float radius, const SWRenderBoundI *crop, SWRenderColor color);
    extern void SWRender_FillRing(SWRenderBuffer *buffer, SWRenderVec2F center, float radius, float thickness, const SWRenderBoundI *crop, SWRenderColor color);
    extern void SWRender_FillRectRounded(SWRenderBuffer *buffer, const SWRenderBoundF *bounds, float corner_radius, SWRenderColor color);

    // line
    extern void SWRender_Line(SWRenderBuffer *buffer, const SWRenderBoundF *line_pos, SWRenderColor color, float thickness);

    // copy
    extern void SWRender_CopyBuffer(const SWRenderBuffer *src, const SWRenderBoundI *src_crop, SWRenderBuffer *dst, const SWRenderVec2I dst_pos_up_left);
    extern void SWRender_CopyBufferScaled(const SWRenderBuffer *src, const SWRenderBoundI *src_crop, SWRenderBuffer *dst, const SWRenderBoundI *dst_pos);

#ifdef __cplusplus
}
#endif
