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

#include "FTEssentials.h"

#include <stdbool.h>
#include <stdint.h>

bool FTEssentials_InitState(FTEssentials_State *state, const char *fontpath)
{
    FT_Error error;
    
    error = FT_Init_FreeType(&state->ft_lib);
    if (error)
    {
        printf("failed to initialize FreeType library with code %d\n", error);
        return false;
    }

    error = FT_New_Face(state->ft_lib, fontpath, 0, &state->ft_face);
    if (error)
    {
        printf("failed to load FreeType face with code: %d\n", error);
        goto ErrorAfterInitFTLib;
    }

    FT_Select_Charmap(state->ft_face, FT_ENCODING_UNICODE);

    state->linespace_multiplier = 1.5F;

    return true;

ErrorAfterInitFTLib:

    FT_Done_FreeType(state->ft_lib);

    return false;
}

void FTEssentials_Done(FTEssentials_State *state)
{
    FT_Done_Face(state->ft_face);
    FT_Done_FreeType(state->ft_lib);
}

FT_ULong FTEssentials_GetUTF32FromCharPtr(const unsigned char *ptr_char, int *char_len_out)
{
    uint64_t charcode;

    const uint8_t utf8_len2_mask = 0b11100000;
    const uint8_t utf8_len3_mask = 0b11110000;
    const uint8_t utf8_len4_mask = 0b11111000;

    const uint8_t utf8_len2_bits = 0b11000000;
    const uint8_t utf8_len3_bits = 0b11100000;
    const uint8_t utf8_len4_bits = 0b11110000;

    const uint8_t utf8_byte_mask = 0b00111111;

    if (ptr_char[0] < 0x80)
    {
        *char_len_out = 1;

        charcode = (uint64_t)ptr_char[0];
    }
    else if ((ptr_char[0] & utf8_len2_mask) == utf8_len2_bits)
    {
        *char_len_out = 2;

        charcode =
            (((uint64_t)(ptr_char[0] & (~utf8_len2_mask))) <<  6) |
            (((uint64_t)(ptr_char[1] &  (utf8_byte_mask))) <<  0);
    }   
    else if ((ptr_char[0] & utf8_len3_mask) == utf8_len3_bits)
    {
        *char_len_out = 3;

        charcode =
            (((uint64_t)(ptr_char[0] & (~utf8_len3_mask))) << 12) |
            (((uint64_t)(ptr_char[1] &  (utf8_byte_mask))) <<  6) |
            (((uint64_t)(ptr_char[2] &  (utf8_byte_mask))) <<  0);
    }
    else if ((ptr_char[0] & utf8_len4_mask) == utf8_len4_bits)
    {
        *char_len_out = 4;
        
        charcode =
            (((uint64_t)(ptr_char[0] & (~utf8_len4_mask))) << 18) |
            (((uint64_t)(ptr_char[1] &  (utf8_byte_mask))) << 12) |
            (((uint64_t)(ptr_char[2] &  (utf8_byte_mask))) <<  6) |
            (((uint64_t)(ptr_char[3] &  (utf8_byte_mask))) <<  0);
    }
    else
    {
        *char_len_out = 1;  // avoid infinity loop

        charcode = 0UL;
    }

    return (FT_ULong)charcode;
}

bool FTEssentials_LoadGlyph(FTEssentials_State *state, FT_ULong charcode)
{
    FT_Error error;

    error = FT_Load_Glyph(state->ft_face, FT_Get_Char_Index(state->ft_face, charcode), FT_LOAD_DEFAULT);
    if (error)
    {
        printf("failed to load glyph in FreeType with code: %d\n", error);
        return false;
    }

    return true;
}

bool FTEssentials_RenderGlyph(FTEssentials_State *state, FT_ULong charcode)
{
    FT_Error error;

    error = FT_Load_Glyph(state->ft_face, FT_Get_Char_Index(state->ft_face, charcode), FT_LOAD_DEFAULT);
    if (error)
    {
        printf("failed to load glyph in FreeType with code: %d\n", error);
        return false;
    }

    error = FT_Render_Glyph(state->ft_face->glyph, FT_RENDER_MODE_NORMAL);
    if (error)
    {
        printf("failed to render glyph in FreeType with code: %d\n", error);
        return false;
    }

    return true;
}

void FTEssentials_GetStrBounds(FTEssentials_State *state, const char *str, FT_F26Dot6 *out_top, FT_F26Dot6 *out_left, FT_F26Dot6 *out_right, FT_F26Dot6 *out_bottom, bool fixed_advance)
{
    FT_Face face = state->ft_face;

    FT_F26Dot6 glyph_x = 0;
    FT_F26Dot6 glyph_y = 0;

    FT_F26Dot6 advance_x;
    if (fixed_advance)
        advance_x = face->size->metrics.max_advance;

    FT_F26Dot6 size_y = face->size->metrics.height;

    FT_F26Dot6 min_x = 0;
    FT_F26Dot6 min_y = 0;
    FT_F26Dot6 max_x = 0;
    FT_F26Dot6 max_y = 0;

    FT_BBox glyph_bbox;

    FT_ULong charcode_last = 0;
    
    bool has_kerning = FT_HAS_KERNING(state->ft_face);

    FT_F26Dot6 linespace = (FT_F26Dot6)((float)(face->size->metrics.ascender - face->size->metrics.descender) * state->linespace_multiplier);

    while (*str != 0)
    {
        int char_len;

        FT_ULong charcode = FTEssentials_GetUTF32FromCharPtr((const unsigned char *)str, &char_len);

        /* newline detection & ptr advance */
        bool newline = false;
        
        if (charcode == '\r')
        {
            if (*(str + 1) == '\n')
                str++;
            
            newline = true;
        }

        if (charcode == '\n')
            newline = true;

        str += char_len;

        if (newline)
        {
            glyph_x = 0;
            // glyph_y += face->size->metrics.height;
            glyph_y += linespace;

            charcode_last = 0;
            
            continue;
        }

        /* kerning adjustments */

        if (has_kerning)
        {
            FT_Vector ft_vec;

            FT_Get_Kerning(face, charcode_last, charcode, FT_KERNING_DEFAULT, &ft_vec);

            glyph_x += ft_vec.x;
        }

        charcode_last = charcode;

        /* glyph loading */
        FTEssentials_LoadGlyph(state, charcode);
        
        FT_GlyphSlot glyph = state->ft_face->glyph;
        FT_Glyph_Metrics *glyph_metrics = &glyph->metrics;

        FT_F26Dot6 this_glyph_x1, this_glyph_y1, this_glyph_x2, this_glyph_y2;
        this_glyph_x1 = glyph_x + glyph_metrics->horiBearingX;
        // this_glyph_y1 = glyph_y - glyph_metrics->horiBearingY;
        // this_glyph_y2 = this_glyph_y1 - face->descender;
        this_glyph_y1 = glyph_y - face->size->metrics.ascender;
        this_glyph_y2 = glyph_y - face->size->metrics.descender;
        if (!fixed_advance)
        {
            this_glyph_x2 = this_glyph_x1 + glyph_metrics->width;
            advance_x = glyph->advance.x;
        }
        else
            this_glyph_x2 = this_glyph_x1 + advance_x;

        if (this_glyph_x1 < min_x)
            min_x = this_glyph_x1;
        if (this_glyph_y1 < min_y)
            min_y = this_glyph_y1;
        if (this_glyph_x2 > max_x)
            max_x = this_glyph_x2;
        if (this_glyph_y2 > max_y)
            max_y = this_glyph_y2;

        glyph_x += advance_x;
    }

    *out_left   = min_x;
    *out_top    = min_y;
    *out_right  = max_x;
    *out_bottom = max_y;
}

void FTEssentials_CalcRenderOffsetByAlignment(FTEssentials_State *state, const char *str, int8_t align_horz, int8_t align_vert, FT_F26Dot6 *offset_x, FT_F26Dot6 *offset_y, bool fixed_advance)
{
    FT_F26Dot6 left, top, right, bottom;
    
    FTEssentials_GetStrBounds(state, str, &top, &left, &right, &bottom, fixed_advance);

    FT_F26Dot6 align_offset_x, align_offset_y;

    if (align_horz < 0)
        align_offset_x = left;
    else if (align_horz > 0)
        align_offset_x = right;
    else
        align_offset_x = (left + right) >> 1;

    if (align_vert < 0)
        align_offset_y = top;
    else if (align_vert > 0)
        align_offset_y = bottom;
    else
        align_offset_y = (top + bottom) >> 1;

    *offset_x = -align_offset_x;
    *offset_y = -align_offset_y;
}

void FTEssentials_SetSize96DPI(FTEssentials_State *state, FT_F26Dot6 size)
{
    FT_Set_Char_Size(state->ft_face, 0, size, 0, 96);
}
