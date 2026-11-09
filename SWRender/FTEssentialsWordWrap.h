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
#include <stddef.h>

#include "FTEssentials.h"

typedef struct FTEssentialsWordWrap_LineStruct
{
    size_t offset;
    size_t bytes;

    FT_F26Dot6 bound_top;
    FT_F26Dot6 bound_left;
    FT_F26Dot6 bound_right;
    FT_F26Dot6 bound_bottom;
}
FTEssentialsWordWrap_Line;

typedef struct FTEssentialsWordWrap_LineBufferStruct
{
    FTEssentialsWordWrap_Line *lines;
    size_t size;
    size_t count;

    FT_F26Dot6 linespace;

    char *buf_brks;
    size_t buf_brks_size;
}
FTEssentialsWordWrap_LineBuffer;

#ifdef __cplusplus
extern "C"
{
#endif

    extern size_t FTEssentialsWordWrap_LineBuffer_init_size;
    extern size_t FTEssentialsWordWrap_LineBuffer_size_min_increment;

    extern bool FTEssentialsWordWrap_LineBufferInit(FTEssentialsWordWrap_LineBuffer *buffer);
    extern void FTEssentialsWordWrap_LineBufferCleanup(FTEssentialsWordWrap_LineBuffer *buffer);

    extern void FTEssentialsWordWrap_LineBufferClear(FTEssentialsWordWrap_LineBuffer *buffer);

    extern bool FTEssentialsWordWrap_GetLinesWidthLimited(
        FTEssentialsWordWrap_LineBuffer *buffer,
        FTEssentials_State *state,
        const char  *str,
        FT_F26Dot6  max_width,
        const char  *lang,
        bool        fixed_advance
    );

#ifdef __cplusplus
}
#endif
