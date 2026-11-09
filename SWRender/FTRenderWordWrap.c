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

#include "FTRenderWordWrap.h"

#include <stdbool.h>
#include <stdint.h>

#include "FTEssentials.h"
#include "FTRender.h"
#include "SWRender.h"
#include "FTEssentialsWordWrap.h"

bool FTRenderWordWrap_RenderStr(FTEssentials_State *state, FTEssentialsWordWrap_LineBuffer *line_buffer, SWRenderBuffer *buffer, FT_F26Dot6 x, FT_F26Dot6 y, FT_F26Dot6 max_width, const char *str, SWRenderColor color)
{
    if (!(state && line_buffer && buffer))
        return false;

    if (!FTEssentialsWordWrap_GetLinesWidthLimited(line_buffer, state, str, max_width, NULL, false))
        return false;
    
    FT_F26Dot6 cursor_y = y;

    for (int i = 0; i < line_buffer->count; i++)
    {
        FTEssentialsWordWrap_Line *line = &line_buffer->lines[i];

        FTRender_RenderStr(state, buffer, x, cursor_y, str + line->offset, line->bytes, color);

        cursor_y += line_buffer->linespace;
    }

    return true;
}

bool FTRenderWordWrap_RenderStrWithAlign(FTEssentials_State *state, FTEssentialsWordWrap_LineBuffer *line_buffer, SWRenderBuffer *buffer, SWRenderRectF *rect, const char *str, int8_t horz_align, int8_t vert_align, SWRenderColor color)
{
    if (!(state && line_buffer && buffer))
        return false;

    SWRenderBoundF bounds;
    if (rect)
        SWRender_RectToBoundFloat(&bounds, rect);
    else
    {
        SWRenderBoundI bounds_int;
        SWRender_BufferGetBounds(&bounds_int, buffer);

        bounds.x1 = bounds_int.x1;
        bounds.y1 = bounds_int.y1;
        bounds.x2 = bounds_int.x2;
        bounds.y2 = bounds_int.y2;
    }
    
    if (!FTEssentialsWordWrap_GetLinesWidthLimited(line_buffer, state, str, FLOAT_TO_F26DOT6(bounds.x2 - bounds.x1), NULL, false))
        return false;

    size_t last_line_index = line_buffer->count - 1;

    long line;
    long line_dir;
    long line_end;
    FT_F26Dot6 cursor_x;
    FT_F26Dot6 cursor_y;

    if (horz_align < 0)
        cursor_x = FLOAT_TO_F26DOT6(bounds.x1);
    else if (horz_align > 0)
        cursor_x = FLOAT_TO_F26DOT6(bounds.x2);
    else
        cursor_x = FLOAT_TO_F26DOT6((bounds.x1 + bounds.x2) * 0.5F);

    if (vert_align < 0)
    {
        cursor_y = FLOAT_TO_F26DOT6(bounds.y1) - line_buffer->lines[0].bound_top;

        line = 0;
        line_dir = 1;
        line_end = line_buffer->count;
    }
    else if (vert_align > 0)
    {
        cursor_y = FLOAT_TO_F26DOT6(bounds.y1) - (-line_buffer->lines[last_line_index].bound_bottom);

        line = last_line_index;
        line_dir = -1;
        line_end = -1;
    }
    else
    {
        FT_F26Dot6 text_size_y = (last_line_index * line_buffer->linespace)
            + (line_buffer->lines[last_line_index].bound_bottom - line_buffer->lines[0].bound_top);

        cursor_y = FLOAT_TO_F26DOT6((bounds.y1 + bounds.y2) * 0.5F)
            + ((-text_size_y / 2) - line_buffer->lines[0].bound_top);

        line = 0;
        line_dir = 1;
        line_end = line_buffer->count;
    }

    while (line != line_end)
    {
        FTEssentialsWordWrap_Line *text_line = &line_buffer->lines[line];

        FT_F26Dot6 offset_x;
        if (horz_align < 0)
            offset_x = -text_line->bound_left;
        else if (horz_align > 0)
            offset_x = -text_line->bound_right;
        else
            offset_x = -(text_line->bound_left + text_line->bound_right) / 2;

        FTRender_RenderStr(state, buffer, cursor_x + offset_x, cursor_y, str + text_line->offset, text_line->bytes, color);

        if (vert_align > 0)
            cursor_y -= line_buffer->linespace;
        else
            cursor_y += line_buffer->linespace;

        line += line_dir;
    }

    return true;
}
