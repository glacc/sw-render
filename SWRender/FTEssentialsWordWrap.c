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

#include "FTEssentialsWordWrap.h"

#include <malloc.h>
#include <stdbool.h>

#include <linebreak.h>

#include "FTEssentials.h"

size_t FTEssentialsWordWrap_LineBuffer_init_size = 8;
size_t FTEssentialsWordWrap_LineBuffer_size_min_increment = 8;
static volatile bool FTEssentialsWordWrap_linebreak_inited = false;

static inline bool FTEssentialsWordWrap_LineBufferCheck(FTEssentialsWordWrap_LineBuffer *buffer)
{
    return (buffer && buffer->lines);
}

bool FTEssentialsWordWrap_LineBufferInit(FTEssentialsWordWrap_LineBuffer *buffer)
{
    if (!buffer)
        return false;

    buffer->lines = malloc(FTEssentialsWordWrap_LineBuffer_init_size * sizeof(*buffer->lines));
    if (!buffer->lines)
    {
        buffer->size  = 0;
        buffer->count = 0;
        return false;
    }
    buffer->size  = FTEssentialsWordWrap_LineBuffer_init_size;
    buffer->count = 0;

    buffer->buf_brks = NULL;
    buffer->buf_brks_size = 0;

    return true;
}

void FTEssentialsWordWrap_LineBufferCleanup(FTEssentialsWordWrap_LineBuffer *buffer)
{
    if (!buffer)
        return;

    if (buffer->lines)
    {
        free(buffer->lines);
        buffer->lines = NULL;
    }
    buffer->size  = 0;
    buffer->count = 0;

    if (buffer->buf_brks)
    {
        free(buffer->buf_brks);
        buffer->buf_brks = NULL;
    }
    buffer->buf_brks_size = 0;
}

void FTEssentialsWordWrap_LineBufferClear(FTEssentialsWordWrap_LineBuffer *buffer)
{
    if (!FTEssentialsWordWrap_LineBufferCheck(buffer))
        return;

    buffer->count = 0;
}

static char FTEssentialsWordWrap_GetTextBoundsInternal(
    FTEssentials_State *state,
    const char  *str,
    const char  *brks,
    FT_ULong    last_charcode,
    FT_F26Dot6  cursor_x,
    FT_F26Dot6  max_width,
    bool        fixed_advance,
    FT_F26Dot6  *out_left,
    FT_F26Dot6  *out_top,
    FT_F26Dot6  *out_right,
    FT_F26Dot6  *out_bottom,
    size_t      *advance_bytes,
    FT_ULong    *out_last_charcode,
    FT_F26Dot6  *out_cursor_x,
    bool        *out_overflow
)
{
    FT_Face face = state->ft_face;

    //FT_F26Dot6 cursor_x = 0;

    FT_F26Dot6 advance_x;
    if (fixed_advance)
        advance_x = face->size->metrics.max_advance;

    FT_F26Dot6 size_y = face->size->metrics.height;

    FT_F26Dot6 min_x = *out_left;
    FT_F26Dot6 min_y = *out_top;
    FT_F26Dot6 max_x = *out_right;
    FT_F26Dot6 max_y = *out_bottom;

    FT_BBox glyph_bbox;

    FT_ULong charcode_last = last_charcode;

    bool has_kerning = FT_HAS_KERNING(state->ft_face);

    size_t bytes = 0;

    char brk = LINEBREAK_NOBREAK;
    bool overflow = false;

    while (*str)
    {
        int char_len;

        FT_ULong charcode = FTEssentials_GetUTF32FromCharPtr((const unsigned char *)str, &char_len);
        brk = *(brks + char_len - 1);
        
        /* kerning adjustments */

        if (has_kerning)
        {
            FT_Vector ft_vec;

            FT_Get_Kerning(face, charcode_last, charcode, FT_KERNING_DEFAULT, &ft_vec);

            cursor_x += ft_vec.x;
        }

        charcode_last = charcode;

        /* glyph loading */
        FTEssentials_LoadGlyph(state, charcode);
        
        FT_GlyphSlot glyph = state->ft_face->glyph;
        FT_Glyph_Metrics *glyph_metrics = &glyph->metrics;

        FT_F26Dot6 this_glyph_x1, this_glyph_y1, this_glyph_x2, this_glyph_y2;
        this_glyph_x1 = cursor_x + glyph_metrics->horiBearingX;
        this_glyph_y1 = -face->size->metrics.ascender;
        this_glyph_y2 = -face->size->metrics.descender;
        if (!fixed_advance)
        {
            this_glyph_x2 = this_glyph_x1 + glyph_metrics->width;
            advance_x = glyph->advance.x;
        }
        else
            this_glyph_x2 = this_glyph_x1 + advance_x;

        /*
        str  += char_len;
        brks += char_len;

        bytes += char_len;
        */

        if (this_glyph_x2 > max_width)
            overflow = true;
        
        if (!overflow || (overflow && cursor_x == 0))
        {
            str  += char_len;
            brks += char_len;

            bytes += char_len;
        }
        
        if (overflow)
            break;

        if (!(FTEssentials_IsWhiteSpace(charcode) && cursor_x == 0))
        {
            if (this_glyph_x1 < min_x)
                min_x = this_glyph_x1;
            if (this_glyph_y1 < min_y)
                min_y = this_glyph_y1;
            if (this_glyph_x2 > max_x)
                max_x = this_glyph_x2;
            if (this_glyph_y2 > max_y)
                max_y = this_glyph_y2;
        }

        cursor_x += advance_x;

        if (brk == LINEBREAK_ALLOWBREAK ||
            brk == LINEBREAK_MUSTBREAK  ||
            brk == LINEBREAK_INDETERMINATE)
            break;
    }

    *out_left   = min_x;
    *out_top    = min_y;
    *out_right  = max_x;
    *out_bottom = max_y;

    *advance_bytes = bytes;
    *out_last_charcode = charcode_last;

    *out_cursor_x = cursor_x;

    *out_overflow = overflow;

    return brk;
}

static bool FTEssentialsWordWrap_AddLineInternal(
    FTEssentialsWordWrap_LineBuffer *buffer,
    size_t offset,
    size_t bytes,
    FT_F26Dot6 bound_left,
    FT_F26Dot6 bound_top,
    FT_F26Dot6 bound_right,
    FT_F26Dot6 bound_bottom
)
{
    if (buffer->count == buffer->size)
    {
        size_t size_increment = buffer->size / 2;
        if (size_increment < FTEssentialsWordWrap_LineBuffer_size_min_increment)
            size_increment = FTEssentialsWordWrap_LineBuffer_size_min_increment;

        size_t new_size = buffer->size + size_increment;

        FTEssentialsWordWrap_Line *lines_new = realloc(buffer->lines, sizeof(*lines_new) * new_size);

        if (!lines_new)
            return false;

        buffer->size  = new_size;
        buffer->lines = lines_new;
    }

    FTEssentialsWordWrap_Line *new_line = &buffer->lines[buffer->count];
    new_line->offset = offset;
    new_line->bytes  = bytes;
    new_line->bound_left   = bound_left;
    new_line->bound_top    = bound_top;
    new_line->bound_right  = bound_right;
    new_line->bound_bottom = bound_bottom;

    buffer->count++;

    return true;
}

bool FTEssentialsWordWrap_GetLinesWidthLimited(FTEssentialsWordWrap_LineBuffer *buffer, FTEssentials_State *state, const char *str, FT_F26Dot6 max_width, const char *lang, bool fixed_advance)
{
    if (!FTEssentialsWordWrap_LineBufferCheck(buffer))
        return false;

    if (!str)
        return false;

    /* preparation */

    if (!FTEssentialsWordWrap_linebreak_inited)
    {
        init_linebreak();
        FTEssentialsWordWrap_linebreak_inited = true;
    }

    FTEssentialsWordWrap_LineBufferClear(buffer);

    size_t str_size = strlen(str);
    if (str_size == 0)
        return true;

    if (str_size > buffer->buf_brks_size)
    {
        char *buf_brk_new = malloc(str_size * sizeof(*buf_brk_new));

        if (!buf_brk_new)
            return false;

        if (buffer->buf_brks)
            free(buffer->buf_brks);

        buffer->buf_brks = buf_brk_new;
    }

    set_linebreaks_utf8((const utf8_t *)str, str_size, lang, buffer->buf_brks);

    buffer->linespace = (FT_F26Dot6)((float)(state->ft_face->size->metrics.ascender - state->ft_face->size->metrics.descender) * state->linespace_multiplier);

    /* calculate lines */

    const char *ptr_str = str;

    FT_F26Dot6 cursor_x = 0;
    FT_ULong last_charcode = 0;

    size_t line_bytes = 0;

    FT_F26Dot6 line_min_x = 0;
    FT_F26Dot6 line_min_y = 0;
    FT_F26Dot6 line_max_x = 0;
    FT_F26Dot6 line_max_y = 0;

    const char *brks = buffer->buf_brks;

    size_t line_start_offset = 0;

    char brk_last = LINEBREAK_NOBREAK;

    while (*ptr_str)
    {
        /* calculate text width for current line */
        size_t advance_bytes;

        bool overflow;

        FT_F26Dot6 min_x = 0;
        FT_F26Dot6 min_y = 0;
        FT_F26Dot6 max_x = 0;
        FT_F26Dot6 max_y = 0;

        char brk_curr = FTEssentialsWordWrap_GetTextBoundsInternal(
            state,
            ptr_str,
            brks,
            last_charcode,
            cursor_x,
            max_width,
            fixed_advance,
            &min_x,
            &min_y,
            &max_x,
            &max_y,
            &advance_bytes,
            &last_charcode,
            &cursor_x,
            &overflow
        );

        const char *ptr_str_next = ptr_str + advance_bytes;
        bool str_ended = (*ptr_str_next == '\0');

        bool advance   = true;
        bool linebreak = true;

        //  brk_last    brk          -> advance     linebreak     //
        //  ----------------------------------------------------  //
        //  X           ALLOWBREAK   -> true        false         //
        //  ALLOWBREAK  NOBREAK      -> false       true          //
        //  NOBREAK     NOBREAK      -> true        true          //
        //  X           MUSTBREAK    -> true        true          //

        if (brk_curr == LINEBREAK_INDETERMINATE)
            brk_curr = LINEBREAK_ALLOWBREAK;
        
        if (brk_curr == LINEBREAK_MUSTBREAK)
        {
            advance   = true;
            linebreak = true;
        }

        if (brk_last == LINEBREAK_NOBREAK && brk_curr == LINEBREAK_NOBREAK)
        {
            advance   = true;
            linebreak = true;
        }

        if (brk_curr == LINEBREAK_ALLOWBREAK)
        {
            advance   = true;
            linebreak = false;
        }

        if (brk_last == LINEBREAK_ALLOWBREAK && brk_curr == LINEBREAK_NOBREAK)
        {
            advance   = false;
            linebreak = true;
        }

        // special overflow condition
        if (brk_last == LINEBREAK_ALLOWBREAK && brk_curr == LINEBREAK_ALLOWBREAK && overflow)
        {
            advance   = false;
            linebreak = true;
        }

        if (advance)
        {
            line_min_x = min_x;
            line_min_y = min_y;
            line_max_x = max_x;
            line_max_y = max_y;
        }

        brk_last = brk_curr;

        if (advance || str_ended)
        {
            ptr_str = ptr_str_next;
            brks += advance_bytes;

            line_bytes += advance_bytes;
        }

        if (linebreak || str_ended)
        {
            bool add_line_succeed = FTEssentialsWordWrap_AddLineInternal(
                buffer,
                line_start_offset,
                (ptr_str - str) - line_start_offset,
                line_min_x,
                line_min_y,
                line_max_x,
                line_max_y
            );
            if (!add_line_succeed)
                return false;

            // update line start offset
            line_start_offset = ptr_str - str;

            // reset state
            cursor_x = 0;
            last_charcode = 0;

            line_min_x = 0;
            line_min_y = 0;
            line_max_x = 0;
            line_max_y = 0;

            line_bytes = 0;

            brk_last = LINEBREAK_NOBREAK;
        }
    }

    return true;
}
