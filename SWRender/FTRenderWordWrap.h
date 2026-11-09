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

#include "FTEssentials.h"
#include "FTRender.h"
#include "SWRender.h"
#include "FTEssentialsWordWrap.h"

#ifdef __cplusplus
extern "C"
{
#endif

    extern bool FTRenderWordWrap_RenderStr(FTEssentials_State *state, FTEssentialsWordWrap_LineBuffer *line_buffer, SWRenderBuffer *buffer, FT_F26Dot6 x, FT_F26Dot6 y, FT_F26Dot6 max_width, const char *str, SWRenderColor color);
    extern bool FTRenderWordWrap_RenderStrWithAlign(FTEssentials_State *state, FTEssentialsWordWrap_LineBuffer *line_buffer, SWRenderBuffer *buffer, SWRenderRectF *rect, const char *str, int8_t horz_align, int8_t vert_align, SWRenderColor color);

#ifdef __cplusplus
}
#endif
