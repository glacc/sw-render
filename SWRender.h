#pragma once

#include <stdint.h>
#include <float.h>

#define SWRENDER_ALIGNMENT 4

#define SWRENDER_LINE_ANTIALIAS_MULT 8
#define SWRENDER_LINE_ANTIALIAS_DIST 1.0F

#define SWRENDER_LINE_BLOCK_SIZE_MIN 8
#define SWRENDER_LINE_BLOCK_SIZE_MAX 512

#define SWRENDER_EPSILON FLT_EPSILON

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
}
SWRenderColor;

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

    extern void SWRender_CheckAndSwapBoundCornersInt(SWRenderBoundI *bounds);
    extern void SWRender_CheckAndSwapBoundCornersFloat(SWRenderBoundF *bounds);

    extern float SWRender_Vec2FLen(SWRenderVec2F vec);
    extern float SWRender_DistPtLineSeg(SWRenderVec2F point, const SWRenderBoundF *line);

    extern void SWRender_AlphaBlendRGBA8888(SWRenderColor src, SWRenderColor *dst);

    extern int SWRender_BufferAlloc(SWRenderBuffer *buffer);
    extern void SWRender_BufferFree(SWRenderBuffer *buffer);

    extern void SWRender_SetPixel(SWRenderBuffer *buffer, SWRenderVec2I pos, SWRenderColor color);
    extern void SWRender_BlendPixel(SWRenderBuffer *buffer, SWRenderVec2I pos, SWRenderColor color);

    extern void SWRender_FillRect(SWRenderBuffer *buffer, const SWRenderBoundI pos, SWRenderColor color, bool overwrite);

    extern void SWRender_Line(SWRenderBuffer *buffer, const SWRenderBoundF *line_pos, SWRenderColor color, float thickness);

    extern void SWRender_CopyBuffer(const SWRenderBuffer *src, const SWRenderBoundI *src_crop, SWRenderBuffer *dst, const SWRenderVec2I dst_pos_up_left);
    extern void SWRender_CopyBufferScaled(const SWRenderBuffer *src, const SWRenderBoundI *src_crop, SWRenderBuffer *dst, const SWRenderBoundI *dst_pos);

#ifdef __cplusplus
}
#endif
