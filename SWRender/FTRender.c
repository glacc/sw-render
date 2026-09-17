#include "FTRender.h"

void FTRender_AlphaBlendARGB8888(SWRenderColor color, uint8_t alpha, SWRenderColor *dst)
{
    uint32_t r_src = color.r;
    uint32_t g_src = color.g;
    uint32_t b_src = color.b;
    uint32_t a_src = (uint32_t)color.a * alpha / 255;

    uint32_t r_dst = dst->r;
    uint32_t g_dst = dst->g;
    uint32_t b_dst = dst->b;
    uint32_t a_dst = dst->a;

    uint32_t a_inv = a_dst * (255 - a_src);

    uint32_t a_new = a_src + (a_inv / 255);
    if (a_new == 0)
        return;
    
    uint32_t a_div = a_new * 255;
    uint32_t a_src_mul_255 = a_src * 255;

    uint32_t r_new = ((r_src * a_src_mul_255) + (r_dst * a_inv)) / a_div;
    uint32_t g_new = ((g_src * a_src_mul_255) + (g_dst * a_inv)) / a_div;
    uint32_t b_new = ((b_src * a_src_mul_255) + (b_dst * a_inv)) / a_div;

    dst->r = r_new;
    dst->g = g_new;
    dst->b = b_new;
    dst->a = a_new;
}

void FTRender_RenderGlyphToSurface(FT_Bitmap *bitmap, SWRenderBuffer *buffer, int x, int y, SWRenderColor color)
{
    int bitmap_w = bitmap->width;
    int bitmap_h = bitmap->rows;
    int surface_w = buffer->w;
    int surface_h = buffer->h;

    int x_start = x;
    int y_start = y;
    int x_end = x + bitmap_w;
    int y_end = y + bitmap_h;

    if (x_end <= 0 || y_end <= 0 || x_start >= surface_w || y_start >= surface_h)
        return;

    int x_offset;
    if (x_start < 0)
    {
        x_offset = -x_start;
        x_start = 0;
    }
    else
        x_offset = 0;
    int y_offset;
    if (y_start < 0)
    {
        y_offset = -y_start;
        y_start = 0;
    }
    else
        y_offset = 0;

    if (x_end >= surface_w)
        x_end = surface_w - 1;
    if (y_end >= surface_h)
        y_end = surface_h - 1;

    int bitmap_pitch = bitmap->pitch;
    uint8_t *ptr_bitmap_row_start = bitmap->buffer;
    if (bitmap_pitch < 0)
        ptr_bitmap_row_start = ptr_bitmap_row_start - ((bitmap_h - 1) * bitmap_pitch);
    ptr_bitmap_row_start += (bitmap_pitch * y_offset) + x_offset;

    int linesize = buffer->linesize;

    SWRenderColor *ptr_row_start = (SWRenderColor *)((uint8_t *)buffer->data + (linesize * y_start)) + x_start;

    for (int pen_y = y_start; pen_y < y_end; pen_y++)
    {
        uint8_t *ptr_bitmap  = ptr_bitmap_row_start;
        SWRenderColor *ptr_pixel = ptr_row_start;
        
        for (int pen_x = x_start; pen_x < x_end; pen_x++)
        {
            FTRender_AlphaBlendARGB8888(color, *ptr_bitmap, ptr_pixel);

            ptr_bitmap++;
            ptr_pixel++;
        }

        ptr_bitmap_row_start  = ptr_bitmap_row_start + bitmap_pitch;
        ptr_row_start = (SWRenderColor *)((uint8_t *)ptr_row_start + linesize);
    }
}

FT_F26Dot6 FTRender_LoadAndRenderGlyph(FTEssentials_State *state, SWRenderBuffer *surface, int x, int y, FT_ULong charcode, SWRenderColor color)
{
    if (!FTEssentials_RenderGlyph(state, charcode))
        return 0;

    FT_GlyphSlot slot = state->ft_face->glyph;
    FT_Bitmap *bitmap = &slot->bitmap;

    int glyph_y = y - slot->bitmap_top;
    int glyph_x = x + slot->bitmap_left;

    FTRender_RenderGlyphToSurface(bitmap, surface, glyph_x, glyph_y, color);

    return slot->advance.x;
}

void FTRender_RenderStr(FTEssentials_State *state, SWRenderBuffer *surface, FT_F26Dot6 x, FT_F26Dot6 y, const char *str, SWRenderColor color)
{
    FT_Face face = state->ft_face;

    FT_F26Dot6 glyph_x_rst = x;
    FT_F26Dot6 glyph_x = glyph_x_rst;
    FT_F26Dot6 glyph_y = y;

    FT_ULong charcode_last = 0;

    bool has_kerning = FT_HAS_KERNING(state->ft_face);

    while (*str != '\0')
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
            glyph_x = glyph_x_rst;
            glyph_y += face->size->metrics.height;

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

        /* render and advance */
        glyph_x += FTRender_LoadAndRenderGlyph(state, surface, glyph_x >> 6, glyph_y >> 6, charcode, color);
    }
}

void FTRender_RenderStrWithAlign(FTEssentials_State *state, SWRenderBuffer *surface, FT_F26Dot6 origin_x, FT_F26Dot6 origin_y, const char *str, int8_t align_horz, int8_t align_vert, SWRenderColor color)
{
    FT_F26Dot6 offset_x, offset_y;
    FTEssentials_CalcRenderOffsetByAlignment(state, str, align_horz, align_vert, &offset_x, &offset_y, false);

    FT_F26Dot6 new_x = origin_x + offset_x;
    FT_F26Dot6 new_y = origin_y + offset_y;

    FTRender_RenderStr(state, surface, new_x, new_y, str, color);
}

void FTRender_RenderCharHorizontallyCentered(FTEssentials_State *state, SWRenderBuffer *surface, FT_F26Dot6 x, FT_F26Dot6 y, FT_ULong charcode, SWRenderColor color)
{
    if (!FTEssentials_RenderGlyph(state, charcode))
        return;

    FT_GlyphSlot slot = state->ft_face->glyph;
    FT_Bitmap *bitmap = &slot->bitmap;
    FT_Glyph_Metrics *glyph_metrics = &slot->metrics;

    int x_render = ((x - (( glyph_metrics->horiBearingX + glyph_metrics->width ) >> 1)) >> 6) + slot->bitmap_left;
    int y_render = (y >> 6) - slot->bitmap_top;

    FTRender_RenderGlyphToSurface(bitmap, surface, x_render, y_render, color);
}

void FTRender_RenderStrFixedAdvance(FTEssentials_State *state, SWRenderBuffer *surface, FT_F26Dot6 x, FT_F26Dot6 y, const char *str, SWRenderColor color)
{
    FT_Face face = state->ft_face;

    FT_F26Dot6 glyph_x_rst = x;
    FT_F26Dot6 glyph_x = glyph_x_rst;
    FT_F26Dot6 glyph_y = y;

    FT_F26Dot6 advance_fixed = state->ft_face->size->metrics.max_advance;

    while (*str != '\0')
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
            glyph_x = glyph_x_rst;
            glyph_y += face->size->metrics.height;
            
            continue;
        }

        FTRender_LoadAndRenderGlyph(state, surface, glyph_x >> 6, glyph_y >> 6, charcode, color);

        glyph_x += advance_fixed;
    }
}

void FTRender_RenderStrFixedAdvanceWithAlign(FTEssentials_State *state, SWRenderBuffer *surface, FT_F26Dot6 origin_x, FT_F26Dot6 origin_y, const char *str, int8_t align_horz, int8_t align_vert, SWRenderColor color)
{
    FT_F26Dot6 offset_x, offset_y;
    FTEssentials_CalcRenderOffsetByAlignment(state, str, align_horz, align_vert, &offset_x, &offset_y, true);

    FT_F26Dot6 new_x = origin_x + offset_x;
    FT_F26Dot6 new_y = origin_y + offset_y;

    FTRender_RenderStrFixedAdvance(state, surface, new_x, new_y, str, color);
}
