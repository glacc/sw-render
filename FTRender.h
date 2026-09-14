#pragma once

#include "FTEssentials.h"

#include "SWRender.h"

#ifdef __cplusplus
extern "C"
{
#endif

    extern void FTRender_AlphaBlendARGB8888(SWRenderColor color, uint8_t alpha, SWRenderColor *dst);
    
    extern void FTRender_RenderGlyphToSurface(FT_Bitmap *bitmap, SWRenderBuffer *buffer, int x, int y, SWRenderColor color);

    FT_F26Dot6 FTRender_LoadAndRenderGlyph(FTEssentials_State *state, SWRenderBuffer *buffer, int x, int y, FT_ULong charcode, SWRenderColor color);

    extern void FTRender_RenderStr(FTEssentials_State *state, SWRenderBuffer *buffer, FT_F26Dot6 x, FT_F26Dot6 y, const char *str, SWRenderColor color);
    extern void FTRender_RenderStrWithAlign(FTEssentials_State *state, SWRenderBuffer *buffer, FT_F26Dot6 origin_x, FT_F26Dot6 origin_y, const char *str, int8_t align_horz, int8_t align_vert, SWRenderColor color);

    extern void FTRender_RenderCharHorizontallyCentered(FTEssentials_State *state, SWRenderBuffer *buffer, FT_F26Dot6 x, FT_F26Dot6 y, FT_ULong charcode, SWRenderColor color);

    extern void FTRender_RenderStrFixedAdvance(FTEssentials_State *state, SWRenderBuffer *buffer, FT_F26Dot6 x, FT_F26Dot6 y, const char *str, SWRenderColor color);
    extern void FTRender_RenderStrFixedAdvanceWithAlign(FTEssentials_State *state, SWRenderBuffer *buffer, FT_F26Dot6 origin_x, FT_F26Dot6 origin_y, const char *str, int8_t align_horz, int8_t align_vert, SWRenderColor color);

#ifdef __cplusplus
}
#endif
