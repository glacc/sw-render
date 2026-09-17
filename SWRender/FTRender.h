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
