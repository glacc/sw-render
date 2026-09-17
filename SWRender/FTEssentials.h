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

#include <stdbool.h>
#include <stdint.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#define INT_TO_F26DOT6(x)       ((x) << 6)
#define FLOAT_TO_F26DOT6(x)     ((FT_F26Dot6)((x) * 64.0F))
#define DOUBLE_TO_F26DOT6(x)    ((FT_F26Dot6)((x) * 64.0))
#define F26DOT6_TO_INT(x)       (((x) + (1 << 5)) >> 6)
#define F26DOT6_TO_FLOAT(x)     ((float)(x) / 64.0F)
#define F26DOT6_TO_DOUBLE(x)    ((double)(x) / 64.0)

typedef struct FTEssentials_StateStruct
{
    FT_Library ft_lib;
    FT_Face ft_face;
}
FTEssentials_State;

#ifdef __cplusplus
extern "C"
{
#endif

    extern bool FTEssentials_InitState(FTEssentials_State *state, const char *fontpath);
    extern void FTEssentials_Done(FTEssentials_State *state);

    extern FT_ULong FTEssentials_GetUTF32FromCharPtr(const unsigned char *ptr_char, int *char_len_out);

    extern bool FTEssentials_LoadGlyph(FTEssentials_State *state, FT_ULong charcode);

    extern bool FTEssentials_RenderGlyph(FTEssentials_State *state, FT_ULong charcode);

    extern void FTEssentials_GetStrBounds(FTEssentials_State *state, const char *str, FT_F26Dot6 *out_top, FT_F26Dot6 *out_left, FT_F26Dot6 *out_right, FT_F26Dot6 *out_bottom, bool fixed_advance);
    extern void FTEssentials_CalcRenderOffsetByAlignment(FTEssentials_State *state, const char *str, int8_t align_horz, int8_t align_vert, FT_F26Dot6 *offset_x, FT_F26Dot6 *offset_y, bool fixed_advance);

    extern void FTEssentials_SetSize96DPI(FTEssentials_State *state, FT_F26Dot6 size);

#ifdef __cplusplus
}
#endif

