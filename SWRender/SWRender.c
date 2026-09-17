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

#include "SWRender.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>

#define __LOG_FILE__ "SWRenderExt.c"
#include "MacroLog.h"

#pragma region CheckAndSwapBoundCorners

static inline void CheckAndSwapBoundCornersInt(int *x1, int *y1, int *x2, int *y2)
{
    int temp;
    if (*x1 > *x2)
    {
        temp = *x1;
        *x1 = *x2;
        *x2 = *x1;
    }
    if (*y1 > *y2)
    {
        temp = *y1;
        *y1 = *y2;
        *y2 = *y1;
    }
}

static inline void CheckAndSwapBoundCornersFloat(float *x1, float *y1, float *x2, float *y2)
{
    float temp;
    if (*x1 > *x2)
    {
        temp = *x1;
        *x1 = *x2;
        *x2 = *x1;
    }
    if (*y1 > *y2)
    {
        temp = *y1;
        *y1 = *y2;
        *y2 = *y1;
    }
}

static inline void SWRender_CheckAndSwapBoundCornersIntInternal(SWRenderBoundI *bounds)
{
    CheckAndSwapBoundCornersInt(&bounds->x1, &bounds->y1, &bounds->x2, &bounds->y2);
}

static inline void SWRender_CheckAndSwapBoundCornersFloatInternal(SWRenderBoundF *bounds)
{
    CheckAndSwapBoundCornersFloat(&bounds->x1, &bounds->y1, &bounds->x2, &bounds->y2);
}

void SWRender_CheckAndSwapBoundCornersInt(SWRenderBoundI *bounds)
{
    if (!bounds)
        return;

    SWRender_CheckAndSwapBoundCornersIntInternal(bounds);
}

void SWRender_CheckAndSwapBoundCornersFloat(SWRenderBoundF *bounds)
{
    if (!bounds)
        return;

    SWRender_CheckAndSwapBoundCornersFloatInternal(bounds);
}

#pragma endregion

#pragma region Vec2F

static inline float SWRender_Vec2FLenInternal(SWRenderVec2F vec)
{
    return sqrtf((vec.x * vec.x) + (vec.y * vec.y));
}

float SWRender_Vec2FLen(SWRenderVec2F vec)
{
    return SWRender_Vec2FLenInternal(vec);
}

#pragma endregion

void SWRender_AlphaBlendRGBA8888(SWRenderColor src, SWRenderColor *dst)
{
    uint32_t r_src = src.r;
    uint32_t g_src = src.g;
    uint32_t b_src = src.b;
    uint32_t a_src = src.a;

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

static int CheckBuffer(const SWRenderBuffer *buffer)
{
    if (!buffer)
        return -1;

    if (!buffer->data)
        return -1;

    return 0;
}

#pragma BufferAllocation

int SWRender_BufferAlloc(SWRenderBuffer *buffer)
{
    if (!buffer)
        return -1;
    
    int w = buffer->w;
    int h = buffer->h;

    if (w <= 0 || h <= 0)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "invalid argument.\n");
        return -1;
    }

    if (!(buffer->data = aligned_alloc(SWRENDER_ALIGNMENT, w * h * 4)))
    {
        fprintf(stderr, LOCATION_PREFIX_STR "failed to allocate memory for buffer.\n");
        return -1;
    }

    buffer->linesize = w * 4;

    return 0;
}

void SWRender_BufferFree(SWRenderBuffer *buffer)
{
    if (CheckBuffer(buffer))
        return;

    free_aligned_sized(buffer->data, SWRENDER_ALIGNMENT, buffer->w * buffer->h * 4);
    buffer->data = NULL;
}

#pragma endregion

static inline SWRenderColor *SWRender_GetPointerByPositionInternal(SWRenderColor *data, int linesize, int x, int y)
{
    return (SWRenderColor *)((uint8_t *)data + (linesize * y)) + x;
}

static inline SWRenderColor *SWRender_GetPointerNextLineInternal(int linesize, SWRenderColor *ptr)
{
    return (SWRenderColor *)((uint8_t *)ptr + linesize);
}

SWRenderColor *SWRender_GetPointerByPosition(const SWRenderBuffer *buffer, int x, int y)
{
    if (CheckBuffer(buffer))
        return NULL;

    if ((x < 0) || (x >= buffer->w))
        return NULL;
    if ((y < 0) || (y >= buffer->h))
        return NULL;

    return SWRender_GetPointerByPositionInternal((SWRenderColor *)buffer->data, buffer->linesize, x, y);
}

SWRenderColor *SWRendet_GetPointerNextLine(const SWRenderBuffer *buffer, SWRenderColor *ptr)
{
    if (CheckBuffer(buffer))
        return NULL;

    SWRenderColor *data = (SWRenderColor *)buffer->data;

    if (ptr < data)
        return NULL;

    int linesize = buffer->linesize;

    SWRenderColor *ptr_max_offset = (SWRenderColor *)((uint8_t *)data + (linesize * (buffer->h - 1)));
    if (ptr >= ptr_max_offset)
        return NULL;

    return SWRender_GetPointerNextLineInternal(linesize, ptr);
}

void SWRender_FillRect(SWRenderBuffer *buffer, const SWRenderBoundI pos, SWRenderColor color, bool overwrite)
{
    if (CheckBuffer(buffer))
        return;

    int x1 = pos.x1;
    int x2 = pos.x2;
    int y1 = pos.y1;
    int y2 = pos.y2;
    int buf_w = buffer->w;
    int buf_h = buffer->h;

    CheckAndSwapBoundCornersInt(&x1, &y1, &x2, &y2);

    if ((x2 < 0) || (x1 >= buf_w) || (y2 < 0) || (y1 >= buf_h))
        return;

    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= buf_w) x2 = buf_w - 1;
    if (y2 >= buf_h) y2 = buf_h - 1;

    bool x1x2_same = (x1 == x2);
    bool y1y2_same = (y1 == y2);

    int linesize = buffer->linesize;
    SWRenderColor *ptr_row_start = SWRender_GetPointerByPositionInternal((SWRenderColor *)buffer->data, linesize, x1, y1);
    SWRenderColor *ptr_pixel;

    // dot?
    if (x1x2_same & y1y2_same)
    {
        if (overwrite)
            *ptr_row_start = color;
        else
            SWRender_AlphaBlendRGBA8888(color, ptr_row_start);
        
        return;
    }

    // vertical line?
    if (x1x2_same)
    {
        for (int y = y1; y <= y2; y++)
        {
            if (overwrite)
                *ptr_row_start = color;
            else
                SWRender_AlphaBlendRGBA8888(color, ptr_row_start);

            ptr_row_start = SWRender_GetPointerNextLineInternal(linesize, ptr_row_start);
        }

        return;
    }

    // horizontal line?
    if (y1y2_same)
    {
        ptr_pixel = ptr_row_start;

        for (int x = x1; x <= x2; x++)
        {
            if (overwrite)
                *ptr_pixel = color;
            else
                SWRender_AlphaBlendRGBA8888(color, ptr_pixel);

            ptr_pixel++;
        }

        return;
    }

    for (int y = y1; y <= y2; y++)
    {
        ptr_pixel = ptr_row_start;

        for (int x = x1; x <= x2; x++)
        {
            if (overwrite)
                *ptr_pixel = color;
            else
                SWRender_AlphaBlendRGBA8888(color, ptr_pixel);

            ptr_pixel++;
        }

        ptr_row_start = SWRender_GetPointerNextLineInternal(linesize, ptr_row_start);
    }
}

/*
// do i really need this?
static bool CheckPointInsideRoundedRect(SWRenderVec2F point, const SWRenderBoundF *bounds, float corner_radius)
{
    float x = point.x;
    float y = point.y;

    float x1 = bounds->x1;
    float y1 = bounds->y1;
    float x2 = bounds->x2;
    float y2 = bounds->y2;

    SWRenderBoundF round_corner_centers =
    {
        x1 + corner_radius,
        y1 + corner_radius,
        x2 - corner_radius,
        y2 - corner_radius,
    };

    if ((y < y1) || (y > y2))
        return false;

    if (x < round_corner_centers.x1)
    {
        if (y < round_corner_centers.y1)
            return (SWRender_Vec2FLen((SWRenderVec2F){ x - round_corner_centers.x1, y - round_corner_centers.y1 }) <= corner_radius);

        if (y > round_corner_centers.y2)
            return (SWRender_Vec2FLen((SWRenderVec2F){ x - round_corner_centers.x1, y - round_corner_centers.y2 }) <= corner_radius);

        return (x >= x1);
    }

    if (x > round_corner_centers.x2)
    {
        if (y < round_corner_centers.y1)
            return (SWRender_Vec2FLen((SWRenderVec2F){ x - round_corner_centers.x2, y - round_corner_centers.y1 }) <= corner_radius);

        if (y > round_corner_centers.y2)
            return (SWRender_Vec2FLen((SWRenderVec2F){ x - round_corner_centers.x2, y - round_corner_centers.y2 }) <= corner_radius);
    }
}
*/

void SWRender_FillCircle(SWRenderBuffer *buffer, SWRenderVec2F center, float radius, const SWRenderBoundI *crop, SWRenderColor color)
{
    if (CheckBuffer(buffer))
        return;

    int x1, y1, x2, y2;
    if (!crop)
    {
        x1 = (int)floorf(center.x - radius);
        y1 = (int)floorf(center.y - radius);
        x2 = (int)ceilf(center.x + radius);
        y2 = (int)ceilf(center.y + radius);
    }
    else
    {
        x1 = crop->x1;
        y1 = crop->y1;
        x2 = crop->x2;
        y2 = crop->y2;
        
        CheckAndSwapBoundCornersInt(&x1, &y1, &x2, &y2);
    }

    int buf_w = buffer->w;
    int buf_h = buffer->h;

    if ((x1 >= buf_w) || (y1 >= buf_h))
        return;
    if ((x2 < 0) || (y2 < 0))
        return;
    
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= buf_w) x2 = buf_w - 1;
    if (y2 >= buf_h) y2 = buf_h - 1;

    SWRenderColor color_aa = color;
    float alpha_original = color_aa.a;

    int linesize = buffer->linesize;
    SWRenderColor *ptr_row_start = SWRender_GetPointerByPositionInternal((SWRenderColor *)buffer->data, linesize, x1, y1);

    float solid_bound = radius - SWRENDER_ANTIALIAS_DIST;
    float antialias_bound = radius + SWRENDER_ANTIALIAS_DIST;

    for (int y = y1; y <= y2; y++)
    {
        SWRenderColor *ptr_pixel = ptr_row_start;

        for (int x = x1; x <= x2; x++)
        {
            float dist = SWRender_Vec2FLenInternal((SWRenderVec2F){ (float)x - center.x + 0.5F, (float)y - center.y + 0.5F });

            if (dist < solid_bound)
                SWRender_AlphaBlendRGBA8888(color, ptr_pixel);
            else if (dist < antialias_bound)
            {
                float average = 0.0F;

                for (int aa_y = 0; aa_y < SWRENDER_ANTIALIAS_MULT; aa_y++)
                {
                    float aa_fine_y = y + ((aa_y + 0.5F) * (1.0F / SWRENDER_ANTIALIAS_MULT));

                    for (int aa_x = 0; aa_x < SWRENDER_ANTIALIAS_MULT; aa_x++)
                    {
                        float aa_fine_x = x + ((aa_x + 0.5F) * (1.0F / SWRENDER_ANTIALIAS_MULT));
                        
                        if (SWRender_Vec2FLenInternal((SWRenderVec2F){ aa_fine_x - center.x, aa_fine_y - center.y }) < radius)
                            average += 1.0F;
                    }
                }

                average *= SWRENDER_ANTIALIAS_DIV_MUL;
                color_aa.a = (uint8_t)(alpha_original* average);

                SWRender_AlphaBlendRGBA8888(color_aa, ptr_pixel);
            }

            ptr_pixel++;
        }

        ptr_row_start = SWRender_GetPointerNextLineInternal(linesize, ptr_row_start);
    }
}

void SWRender_FillRing(SWRenderBuffer *buffer, SWRenderVec2F center, float radius, float thickness, const SWRenderBoundI *crop, SWRenderColor color)
{
    if (CheckBuffer(buffer))
        return;

    int x1, y1, x2, y2;
    if (!crop)
    {
        x1 = (int)floorf(center.x - radius);
        y1 = (int)floorf(center.y - radius);
        x2 = (int)ceilf(center.x + radius);
        y2 = (int)ceilf(center.y + radius);
    }
    else
    {
        x1 = crop->x1;
        y1 = crop->y1;
        x2 = crop->x2;
        y2 = crop->y2;
        
        CheckAndSwapBoundCornersInt(&x1, &y1, &x2, &y2);
    }

    int buf_w = buffer->w;
    int buf_h = buffer->h;

    if ((x1 >= buf_w) || (y1 >= buf_h))
        return;
    if ((x2 < 0) || (y2 < 0))
        return;
    
    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= buf_w) x2 = buf_w - 1;
    if (y2 >= buf_h) y2 = buf_h - 1;

    SWRenderColor color_aa = color;
    float alpha_original = color_aa.a;

    int linesize = buffer->linesize;
    SWRenderColor *ptr_row_start = SWRender_GetPointerByPositionInternal((SWRenderColor *)buffer->data, linesize, x1, y1);

    float antialias_outer_bound = radius + SWRENDER_ANTIALIAS_DIST;
    float solid_outer_bound = radius - SWRENDER_ANTIALIAS_DIST;
    float solid_inner_bound = radius - (thickness - SWRENDER_ANTIALIAS_DIST);
    float antialias_inner_bound = radius - (thickness + SWRENDER_ANTIALIAS_DIST);

    float inner_radius = radius - thickness;

    for (int y = y1; y <= y2; y++)
    {
        SWRenderColor *ptr_pixel = ptr_row_start;

        for (int x = x1; x <= x2; x++)
        {
            float dist = SWRender_Vec2FLenInternal((SWRenderVec2F){ (float)x - center.x + 0.5F, (float)y - center.y + 0.5F });

            bool in_soild_zone     = (dist > solid_inner_bound) && (dist < solid_outer_bound);
            bool in_antialias_zone = (dist > antialias_inner_bound) && (dist < antialias_outer_bound);

            if (in_soild_zone)
                SWRender_AlphaBlendRGBA8888(color, ptr_pixel);
            else if (in_antialias_zone)
            {
                float average = 0.0F;

                for (int aa_y = 0; aa_y < SWRENDER_ANTIALIAS_MULT; aa_y++)
                {
                    float aa_fine_y = y + ((aa_y + 0.5F) * (1.0F / SWRENDER_ANTIALIAS_MULT));

                    for (int aa_x = 0; aa_x < SWRENDER_ANTIALIAS_MULT; aa_x++)
                    {
                        float aa_fine_x = x + ((aa_x + 0.5F) * (1.0F / SWRENDER_ANTIALIAS_MULT));
                        
                        float dist_fine = SWRender_Vec2FLenInternal((SWRenderVec2F){ aa_fine_x - center.x, aa_fine_y - center.y });
                        if ((dist_fine > inner_radius) && (dist_fine < radius))
                            average += 1.0F;
                    }
                }

                average *= SWRENDER_ANTIALIAS_DIV_MUL;
                color_aa.a = (uint8_t)(alpha_original* average);

                SWRender_AlphaBlendRGBA8888(color_aa, ptr_pixel);
            }

            ptr_pixel++;
        }

        ptr_row_start = SWRender_GetPointerNextLineInternal(linesize, ptr_row_start);
    }
}

void SWRender_FillRectRounded(SWRenderBuffer *buffer, const SWRenderBoundF pos, float corner_radius, SWRenderColor color)
{
    if (CheckBuffer(buffer))
        return;
    
    float x1_orig = pos.x1;
    float y1_orig = pos.y1;
    float x2_orig = pos.x2;
    float y2_orig = pos.y2;
    CheckAndSwapBoundCornersFloat(&x1_orig, &y1_orig, &x2_orig, &y2_orig);

    int buf_w = buffer->w;
    int buf_h = buffer->h;

    if ((x1_orig >= buf_w) || (y1_orig >= buf_h))
        return;
    if ((x2_orig < 0.0F) || (y2_orig < 0.0F))
        return;

    // rectangle sizes
    float rect_w = x2_orig - x1_orig;
    float rect_h = y2_orig - y1_orig;

    // limit corner radius
    if (corner_radius <= 0.0F)
        corner_radius = 0.0F;
    else
    {
        float max_corner_diameter = rect_w;
        if (rect_w > rect_h)
            max_corner_diameter = rect_h;

        float max_radius = max_corner_diameter * 0.5F;
        if (corner_radius > max_radius)
            corner_radius = max_radius;
    }

    // round corner center positions
    SWRenderBoundF corner_center_bounds = 
    {
        x1_orig + corner_radius,
        y1_orig + corner_radius,
        x2_orig - corner_radius,
        y2_orig - corner_radius,
    };

    // edge color after anti-aliasing
    float x1_float = x1_orig;
    float y1_float = y1_orig;
    float x4_float = x2_orig;
    float y4_float = y2_orig;
    float x1_floorf = floorf(x1_float);
    float y1_floorf = floorf(y1_float);
    float x4_floorf = floorf(x4_float);
    float y4_floorf = floorf(y4_float);

    int x1 = (int)x1_floorf;
    int y1 = (int)y1_floorf;
    int x4 = (int)x4_floorf;
    int y4 = (int)y4_floorf;
    int x1_next = x1 + 1;
    int y1_next = y1 + 1;
    int x4_next = x4 + 1;
    int y4_next = y4 + 1;

    SWRenderColor color_l_edge = color;
    SWRenderColor color_u_edge = color;
    SWRenderColor color_r_edge = color;
    SWRenderColor color_d_edge = color;

    color_l_edge.a = (uint8_t)((float)color_l_edge.a * ((float)x1_next - x1_floorf));
    color_u_edge.a = (uint8_t)((float)color_u_edge.a * ((float)y1_next - y1_floorf));

    int x1_final = x1;
    int y1_final = y1;
    int x4_final = x4;
    int y4_final = y4;
    
    // x4 & y4 alpha overflow to next pixel
    if (x4_float - x4_floorf > SWRENDER_EPSILON)
    {
        color_r_edge.a = (uint8_t)((float)color_r_edge.a * ((float)x4_next - x4_floorf));
        x4_final++;
    }
    if (y4_float - y4_floorf > SWRENDER_EPSILON)
    {
        color_d_edge.a = (uint8_t)((float)color_d_edge.a * ((float)y4_next - y4_floorf));
        y4_final++;
    }

    int x2_final = (int)floorf(corner_center_bounds.x1);
    int y2_final = (int)floorf(corner_center_bounds.y1);
    int x3_final = (int)floorf(corner_center_bounds.x2);
    int y3_final = (int)floorf(corner_center_bounds.y2);

    // draw
    //     1. ul corner ->  top rect   -> ur corner
    //     2.    ->        middle rect       ->
    //     3. dl corner -> bottom rect -> dr corner
    //              divided into 7 parts
    // adjusted sequence to 1 -> 3 -> 2 to reuse variables

    int x1_draw = x1_final;
    int y1_draw = x1_final;
    int x2_draw = x2_final;
    int y2_draw = y2_final;
    int x3_draw = x3_final;
    int y3_draw = y3_final;
    int x4_draw = x4_final;
    int y4_draw = y4_final;
    
    if (x1_draw < 0) x1_draw = 0;
    if (x2_draw < 0) x2_draw = 0;
    if (x3_draw < 0) x3_draw = 0;
    if (x2_draw >= buf_w) x2_draw = buf_w - 1;
    if (x3_draw >= buf_w) x3_draw = buf_w - 1;
    if (x4_draw >= buf_w) x4_draw = buf_w - 1;

    if (y1_draw < 0) y1_draw = 0;
    if (y2_draw < 0) y2_draw = 0;
    if (y3_draw < 0) y3_draw = 0;
    if (y2_draw >= buf_h) y2_draw = buf_h - 1;
    if (y3_draw >= buf_h) y3_draw = buf_h - 1;
    if (y4_draw >= buf_h) y4_draw = buf_h - 1;

    // ul corner -> top rect -> ur corner
    int x_start, x_end;
    int y_start, y_end;
    SWRenderColor *ptr_row_start;

    int linesize = buffer->linesize;
    SWRenderColor *data = (SWRenderColor *)buffer->data;

    // for 1.x and 2.x
    x_start = (x2_draw == x2_final) ? (x2_draw + 1) : x2_draw;
    x_end   = (x3_draw == x3_final) ? (x3_draw - 1) : x3_draw;

    if (x3_final == x2_final)
        x3_final++;
    if (y3_final == y2_final)
        y3_final++;

    // 1.1 ul corner & 1.2 ur corner
    SWRender_FillCircle(buffer, (SWRenderVec2F){ corner_center_bounds.x1, corner_center_bounds.y1 }, corner_radius, &(SWRenderBoundI){ x1_final, y1_final, x2_final, y2_final }, color);
    SWRender_FillCircle(buffer, (SWRenderVec2F){ corner_center_bounds.x2 + 1.0F, corner_center_bounds.y1 }, corner_radius, &(SWRenderBoundI){ x3_final, y1_final, x4_final, y2_final }, color);
    
    // 1.3 top rect
    y_start = y1_draw;
    y_end   = y2_draw;

    if (y1_draw == y1_final)
    {
        // 1.3.1 anti-aliased top edge
        SWRenderColor *ptr_pixel = SWRender_GetPointerByPositionInternal(data, linesize, x_start, y_start);
        for (int x = x_start; x <= x_end; x++)
        {
            SWRender_AlphaBlendRGBA8888(color_u_edge, ptr_pixel);
            ptr_pixel++;
        }
        y_start++;
    }

    // 1.3.2 top solid part
    if ((x_start <= x_end) && (y_start <= y_end))
        SWRender_FillRect(buffer, (SWRenderBoundI){ x_start, y_start, x_end, y_end }, color, false);

    // 2.1 dl corner & 2.3 dr corner
    SWRender_FillCircle(buffer, (SWRenderVec2F){ corner_center_bounds.x1, corner_center_bounds.y2 + 1.0F }, corner_radius, &(SWRenderBoundI){ x1_final, y3_final, x2_final, y4_final }, color);
    SWRender_FillCircle(buffer, (SWRenderVec2F){ corner_center_bounds.x2 + 1.0F, corner_center_bounds.y2 + 1.0F }, corner_radius, &(SWRenderBoundI){ x3_final, y3_final, x4_final, y4_final }, color);

    // 2.3 bottom rect
    y_start = y3_draw;
    y_end   = y4_draw;

    if (y4_draw == y4_final)
    {
        // 2.3.1 anti-aliased top edge
        SWRenderColor *ptr_pixel = SWRender_GetPointerByPositionInternal(data, linesize, x_start, y_end);
        for (int x = x_start; x <= x_end; x++)
        {
            SWRender_AlphaBlendRGBA8888(color_d_edge, ptr_pixel);
            ptr_pixel++;
        }
        y_end--;
    }

    // 2.3.2 bottom solid part
    if ((x_start <= x_end) && (y_start <= y_end))
        SWRender_FillRect(buffer, (SWRenderBoundI){ x_start, y_start, x_end, y_end }, color, false);

    // 3.1 middle
    x_start = x1_draw;
    x_end   = x4_draw;
    y_start = y2_draw + 1;
    y_end   = y3_draw - 1;

    if (y_start <= y_end)
    {
        if (x1_draw == x1_final)
        {
            // 3.1.1 left edge
            SWRenderColor *ptr_pixel = SWRender_GetPointerByPositionInternal(data, linesize, x_start, y_start);
            for (int y = y_start; y <= y_end; y++)
            {
                SWRender_AlphaBlendRGBA8888(color_l_edge, ptr_pixel);
                ptr_pixel = SWRender_GetPointerNextLineInternal(linesize, ptr_pixel);
            }
            x_start++;
        }

        if (x4_draw == x4_final)
        {
            // 3.1.2 right edge
            SWRenderColor *ptr_pixel = SWRender_GetPointerByPositionInternal(data, linesize, x_end, y_start);
            for (int y = y_start; y <= y_end; y++)
            {
                SWRender_AlphaBlendRGBA8888(color_r_edge, ptr_pixel);
                ptr_pixel = SWRender_GetPointerNextLineInternal(linesize, ptr_pixel);
            }
            x_end--;
        }

        // 3.1.3 center
        SWRender_FillRect(buffer, (SWRenderBoundI){ x_start, y_start, x_end, y_end }, color, false);
    }
}

#pragma region Line

#pragma region LineInternal

static float DistPtLineSeg(SWRenderVec2F point, const SWRenderBoundF *line_pos, float line_length_sq)
{
    SWRenderVec2F vec_line_start = { line_pos->x1, line_pos->y1 };
    SWRenderVec2F vec_line_start_to_pt = { point.x - vec_line_start.x, point.y - vec_line_start.y };

    if (line_length_sq <= .0F)
        return SWRender_Vec2FLenInternal(vec_line_start_to_pt);

    SWRenderVec2F vec_line_end = { line_pos->x2, line_pos->y2 };
    SWRenderVec2F vec_line_seg = { vec_line_end.x - vec_line_start.x, vec_line_end.y - vec_line_start.y };

    float proj_pt_to_line_norm = ((vec_line_start_to_pt.x * vec_line_seg.x) + (vec_line_start_to_pt.y * vec_line_seg.y)) / line_length_sq;

    if (proj_pt_to_line_norm < 0.0F)
        return SWRender_Vec2FLenInternal(vec_line_start_to_pt);
    
    if (proj_pt_to_line_norm > 1.0F)
    {
        SWRenderVec2F vec_line_end_to_pt = { point.x - vec_line_end.x, point.y - vec_line_end.y };
        return SWRender_Vec2FLenInternal(vec_line_end_to_pt);
    }

    SWRenderVec2F vec_proj_pt = { vec_line_start.x + (vec_line_seg.x * proj_pt_to_line_norm), vec_line_start.y + (vec_line_seg.y * proj_pt_to_line_norm )};
    SWRenderVec2F vec_pt_to_proj_pt = { point.x - vec_proj_pt.x, point.y - vec_proj_pt.y };
    return SWRender_Vec2FLenInternal(vec_pt_to_proj_pt);
}

#pragma region LineIntersectionCheck

static inline bool CheckHasIntersectionBetweenLineAndRoundCorner(SWRenderVec2F corner_center, const SWRenderBoundF *line_pos, float line_length_sq, float thickness)
{
    return (DistPtLineSeg(corner_center, line_pos, line_length_sq) <= thickness);
}

static inline float GetTripletOrientation(SWRenderVec2F p1, SWRenderVec2F p2, SWRenderVec2F p3)
{
    return ((p2.y - p1.y) * (p3.x - p2.x)) - ((p2.x - p1.x) * (p3.y - p2.y));
}

static inline bool CheckPointInsideLineBoundingBox(const SWRenderBoundF *line_bounds, SWRenderVec2F point)
{
    return ((point.x >= line_bounds->x1) && (point.y >= line_bounds->x2) && (point.x <= line_bounds->x2) && (point.y <= line_bounds->y2));
}

static bool CheckHasIntersectionBetweenTwoLineSegs(const SWRenderBoundF *line1_pos, const SWRenderBoundF *line2_pos)
{
    SWRenderVec2F line1_pt1 = { line1_pos->x1, line1_pos->y1 };
    SWRenderVec2F line1_pt2 = { line1_pos->x2, line1_pos->y2 };
    SWRenderVec2F line2_pt1 = { line2_pos->x1, line2_pos->y1 };
    SWRenderVec2F line2_pt2 = { line2_pos->x2, line2_pos->y2 };

    float orientation1 = GetTripletOrientation(line1_pt1, line1_pt2, line2_pt1);
    float orientation2 = GetTripletOrientation(line1_pt1, line1_pt2, line2_pt2);
    float orientation3 = GetTripletOrientation(line2_pt1, line2_pt2, line1_pt1);
    float orientation4 = GetTripletOrientation(line2_pt1, line2_pt2, line1_pt2);

    if ((orientation1 * orientation2 < 0.0F) && (orientation3 * orientation4 < 0.0F))
        return true;

    SWRenderBoundF line1_bounds = *line1_pos;
    SWRender_CheckAndSwapBoundCornersFloatInternal(&line1_bounds);
    if ((fabs(orientation1) < FLT_EPSILON) && CheckPointInsideLineBoundingBox(&line1_bounds, line2_pt1))
        return true;
    if ((fabs(orientation2) < FLT_EPSILON) && CheckPointInsideLineBoundingBox(&line1_bounds, line2_pt2))
        return true;

    SWRenderBoundF line2_bounds = *line2_pos;
    SWRender_CheckAndSwapBoundCornersFloatInternal(&line2_bounds);
    if ((fabs(orientation3) < FLT_EPSILON) && CheckPointInsideLineBoundingBox(&line2_bounds, line1_pt1))
        return true;
    if ((fabs(orientation4) < FLT_EPSILON) && CheckPointInsideLineBoundingBox(&line2_bounds, line1_pt2))
        return true;

    return false;
}

static bool CheckLineInsideBlockBoundingBox(const SWRenderBoundF *line_pos, const SWRenderBoundI *bounding_box, float half_thickness)
{
    SWRenderBoundF block_bounding_box_expanded =
    {
        bounding_box->x1 - half_thickness,
        bounding_box->y1 - half_thickness,
        bounding_box->x2 + half_thickness,
        bounding_box->y2 + half_thickness,
    };

    SWRenderBoundF line_bounding_box = *line_pos;
    SWRender_CheckAndSwapBoundCornersFloatInternal(&line_bounding_box);

    return ((line_bounding_box.x1 >= block_bounding_box_expanded.x1) && (line_bounding_box.y1 >= block_bounding_box_expanded.y1) && (line_bounding_box.x2 < block_bounding_box_expanded.x2) && (line_bounding_box.y2 < block_bounding_box_expanded.y2));
}

static bool CheckIntersectionBlockBoundingBox(const SWRenderBoundI *bounds, const SWRenderBoundF *line_pos, float line_length_sq, float thickness)
{
    // corners
    SWRenderVec2F corner_ul = { bounds->x1, bounds->y1 };
    if (CheckHasIntersectionBetweenLineAndRoundCorner(corner_ul, line_pos, line_length_sq, thickness))
        return true;

    SWRenderVec2F corner_ur = { bounds->x2, bounds->y1 };
    if (CheckHasIntersectionBetweenLineAndRoundCorner(corner_ur, line_pos, line_length_sq, thickness))
        return true;
    
    SWRenderVec2F corner_dl = { bounds->x1, bounds->y2 };
    if (CheckHasIntersectionBetweenLineAndRoundCorner(corner_dl, line_pos, line_length_sq, thickness))
        return true;

    SWRenderVec2F corner_dr = { bounds->x2, bounds->y2 };
    if (CheckHasIntersectionBetweenLineAndRoundCorner(corner_dr, line_pos, line_length_sq, thickness))
        return true;

    // edges
    float edge_u_y = bounds->y1 - thickness;
    SWRenderBoundF edge_u = { bounds->x1, edge_u_y, bounds->x2, edge_u_y};
    if (CheckHasIntersectionBetweenTwoLineSegs(&edge_u, line_pos))
        return true;

    float edge_r_x = bounds->x2 + thickness;
    SWRenderBoundF edge_r = { edge_r_x, bounds->y1, edge_r_x, bounds->y2};
    if (CheckHasIntersectionBetweenTwoLineSegs(&edge_r, line_pos))
        return true;

    float edge_d_y = bounds->y2 + thickness;
    SWRenderBoundF edge_d = { bounds->x1, edge_d_y, bounds->x2, edge_d_y};
    if (CheckHasIntersectionBetweenTwoLineSegs(&edge_d, line_pos))
        return true;

    float edge_l_x = bounds->x1 - thickness;
    SWRenderBoundF edge_l = { edge_l_x, bounds->y1, edge_l_x, bounds->y2};
    if (CheckHasIntersectionBetweenTwoLineSegs(&edge_l, line_pos))
        return true;

    return false;
}

#pragma endregion

static void RecursiveBlockedLine(SWRenderBuffer *buffer, SWRenderVec2I *block_pos, int block_size, const SWRenderBoundF *line_pos, float line_length_sq, const SWRenderColor *color, float half_thickness)
{
    int blk_x1 = block_pos->x;
    int blk_y1 = block_pos->y;
    int blk_x2 = blk_x1 + block_size;
    int blk_y2 = blk_y1 + block_size;
    int buf_w = buffer->w;
    int buf_h = buffer->h;

    // outside of screen
    if ((blk_x1 >= buf_w) || blk_y1 >= buf_h)
        return;

    //  check intersection / include
    float half_thickness_with_safezone =half_thickness + SWRENDER_LINE_SAFEZONE_SIZE;
    SWRenderBoundI block_bounds = { blk_x1, blk_y1, blk_x2, blk_y2 };
    if (!(CheckIntersectionBlockBoundingBox(&block_bounds, line_pos, line_length_sq, half_thickness_with_safezone) || CheckLineInsideBlockBoundingBox(line_pos, &block_bounds, half_thickness_with_safezone)))
        return;

    if (block_size > SWRENDER_LINE_BLOCK_SIZE_MIN)
    {
        int half_blk_size = block_size >> 1;

        int blk_xr = blk_x1 + half_blk_size;
        int blk_yd = blk_y1 + half_blk_size;

        RecursiveBlockedLine(buffer, block_pos, half_blk_size, line_pos, line_length_sq, color, half_thickness);
        RecursiveBlockedLine(buffer, &(SWRenderVec2I){ blk_xr, blk_y1 }, half_blk_size, line_pos, line_length_sq, color, half_thickness);
        RecursiveBlockedLine(buffer, &(SWRenderVec2I){ blk_x1, blk_yd }, half_blk_size, line_pos, line_length_sq, color, half_thickness);
        RecursiveBlockedLine(buffer, &(SWRenderVec2I){ blk_xr, blk_yd }, half_blk_size, line_pos, line_length_sq, color, half_thickness);

        return;
    }

    // draw
    bool ul_inside_line = (DistPtLineSeg((SWRenderVec2F){ blk_x1, blk_y1 }, line_pos, line_length_sq) < half_thickness);
    bool ur_inside_line = (DistPtLineSeg((SWRenderVec2F){ blk_x2, blk_y1 }, line_pos, line_length_sq) < half_thickness);
    bool dl_inside_line = (DistPtLineSeg((SWRenderVec2F){ blk_x1, blk_y2 }, line_pos, line_length_sq) < half_thickness);
    bool dr_inside_line = (DistPtLineSeg((SWRenderVec2F){ blk_y1, blk_y2 }, line_pos, line_length_sq) < half_thickness);

    if (blk_x2 > buf_w) blk_x2 = buf_w;
    if (blk_y2 > buf_h) blk_y2 = buf_h;

    int linesize = buffer->linesize;
    SWRenderColor *data = (SWRenderColor *)buffer->data;
    SWRenderColor *ptr_row_start = SWRender_GetPointerByPositionInternal(data, linesize, blk_x1, blk_y1);

    // fill
    if (ul_inside_line && ur_inside_line && dl_inside_line && dr_inside_line)
    {
        for (int y = blk_y1; y < blk_y2; y++)
        {
            SWRenderColor *ptr_pixel = ptr_row_start;
            
            for (int x = blk_x1; x < blk_x2; x++)
            {
                SWRender_AlphaBlendRGBA8888(*color, ptr_pixel);
                ptr_pixel++;
            }

            ptr_row_start = SWRender_GetPointerNextLineInternal(linesize, ptr_row_start);
        }

        return;
    }

    SWRenderColor color_aa = *color;
    float alpha_original = color_aa.a;

    float solid_bound = half_thickness - SWRENDER_ANTIALIAS_DIST;
    float antialias_bound = half_thickness + SWRENDER_ANTIALIAS_DIST;

    for (int y = blk_y1; y < blk_y2; y++)
    {
        SWRenderColor *ptr_pixel = ptr_row_start;
        
        for (int x = blk_x1; x < blk_x2; x++)
        {
            float dist = DistPtLineSeg((SWRenderVec2F){ x + 0.5F, y + 0.5F }, line_pos, line_length_sq);

            if (dist < solid_bound)
                SWRender_AlphaBlendRGBA8888(*color, ptr_pixel);
            else if (dist < antialias_bound)
            {
                float average = 0.0F;

                for (int aa_y = 0; aa_y < SWRENDER_ANTIALIAS_MULT; aa_y++)
                {
                    float aa_fine_y = y + ((aa_y + 0.5F) * (1.0F / SWRENDER_ANTIALIAS_MULT));

                    for (int aa_x = 0; aa_x < SWRENDER_ANTIALIAS_MULT; aa_x++)
                    {
                        float aa_fine_x = x + ((aa_x + 0.5F) * (1.0F / SWRENDER_ANTIALIAS_MULT));
                        
                        if (DistPtLineSeg((SWRenderVec2F){ aa_fine_x, aa_fine_y }, line_pos, line_length_sq) < half_thickness)
                            average += 1.0F;
                    }
                }

                average *= SWRENDER_ANTIALIAS_DIV_MUL;
                color_aa.a = (uint8_t)(alpha_original * average);

                SWRender_AlphaBlendRGBA8888(color_aa, ptr_pixel);
            }

            ptr_pixel++;
        }

        ptr_row_start = SWRender_GetPointerNextLineInternal(linesize, ptr_row_start);
    }
}

#pragma endregion

float SWRender_DistPtLineSeg(SWRenderVec2F point, const SWRenderBoundF *line)
{
    SWRenderVec2F line_vec = { line->x2 - line->x1, line->y2 - line->y1 };
    return DistPtLineSeg(point, line, (line_vec.x * line_vec.x) + (line_vec.y * line_vec.y));
}

void SWRender_Line(SWRenderBuffer *buffer, const SWRenderBoundF *line_pos, SWRenderColor color, float thickness)
{
    if (CheckBuffer(buffer))
        return;

    if (thickness <= 0)
        return;

    // check
    float chk_x1 = line_pos->x1;
    float chk_y1 = line_pos->y1;
    float chk_x2 = line_pos->x2;
    float chk_y2 = line_pos->y2;
    int buf_w = buffer->w;
    int buf_h = buffer->h;

    CheckAndSwapBoundCornersFloat(&chk_x1, &chk_y1, &chk_x2, &chk_y2);

    float chk_lt_bound = -thickness;
    float chk_r_bound = buf_w + thickness;
    float chk_b_bound = buf_h + thickness;

    if ((chk_x1 < chk_lt_bound) || (chk_x2 > chk_r_bound))
        return;
    if ((chk_y1 < chk_lt_bound) || (chk_y2 > chk_b_bound))
        return;

    float half_thickness = thickness / 2.0F;
    float line_length_sq = SWRender_Vec2FLenInternal((SWRenderVec2F){ line_pos->x2 - line_pos->x1, line_pos->y2 - line_pos->y1 });
    line_length_sq *= line_length_sq;

    for (int y = 0; y < buf_h; y += SWRENDER_LINE_BLOCK_SIZE_MAX)
    {
        for (int x = 0; x < buf_w; x += SWRENDER_LINE_BLOCK_SIZE_MAX)
            RecursiveBlockedLine(buffer, &(SWRenderVec2I){ x, y }, SWRENDER_LINE_BLOCK_SIZE_MAX, line_pos, line_length_sq, &color, half_thickness);
    }
}

#pragma endregion

#pragma region CopyBuffer

void SWRender_CopyBuffer(const SWRenderBuffer *src, const SWRenderBoundI *src_crop, SWRenderBuffer *dst, const SWRenderVec2I dst_pos_up_left)
{
    if (CheckBuffer(src) || CheckBuffer(dst))
        return;

    // source
    int src_x1, src_y1, src_x2, src_y2;
    int src_w = src->w;
    int src_h = src->h;

    if (src_crop)
    {
        src_x1 = src_crop->x1;
        src_y1 = src_crop->y1;
        src_x2 = src_crop->x2;
        src_y2 = src_crop->y2;
    }
    else
    {
        src_x1 = src_y1 = 0;
        src_x2 = src_w - 1;
        src_y2 = src_h - 1;
    }
    
    CheckAndSwapBoundCornersInt(&src_x1, &src_y1, &src_x2, &src_y2);

    // destination
    int dst_x1 = dst_pos_up_left.x;
    int dst_y1 = dst_pos_up_left.y;
    int dst_x2;
    int dst_y2;
    int dst_w = dst->w;
    int dst_h = dst->h;

    // invalid crop check
    if ((src_x1 >= src_w) || (src_y1 >= src_h))
        return;
    if ((src_x2 < 0) || (src_y2 < 0))
        return;

    // to myself below: i use dst_x2 & dst_y2 as right & bottom edge so src_x2 & src_y2 is almost not used.

    // source offset out-of-bound crop & bound_check
    if (src_x1 < 0)
    {
        dst_x1 -= src_x1;
        src_x1 = 0;
    }
    if (src_y1 < 0)
    {
        dst_y1 -= src_y1;
        src_y1 = 0;
    }
    if (src_x2 >= src_w) src_x2 = src_w - 1;
    if (src_y2 >= src_h) src_y2 = src_h - 1;

    int src_w_after_crop = src_x2 - src_x1 + 1;
    int src_h_after_crop = src_y2 - src_y1 + 1;
    
    // destination bound check
    dst_x2 = dst_x1 + src_w_after_crop - 1;
    dst_y2 = dst_y1 + src_h_after_crop - 1;

    if ((dst_x2 < 0) || (dst_y2 < 0))
        return;
    if ((dst_x1 >= dst_w) || (dst_y1 >= dst_h))
        return;

    if (dst_x2 >= dst_w) dst_x2 = dst_w - 1;
    if (dst_y2 >= dst_h) dst_y2 = dst_h - 1;

    if (dst_x1 < 0)
    {
        src_x1 -= dst_x1;
        dst_x1 = 0;
    }
    if (dst_y1 < 0)
    {
        src_y1 -= dst_y1;
        dst_y1 = 0;
    }
    
    // pointers
    int linesize_src = src->linesize;
    int linesize_dst = dst->linesize;
    SWRenderColor *ptr_src_row_start = (SWRenderColor *)((uint8_t *)src->data + (src_y1 * linesize_src)) + src_x1;
    SWRenderColor *ptr_dst_row_start = (SWRenderColor *)((uint8_t *)dst->data + (dst_y1 * linesize_dst)) + dst_x1;

    for (int dst_y = dst_y1; dst_y <= dst_y2; dst_y++)
    {
        SWRenderColor *ptr_src_pixel = ptr_src_row_start;
        SWRenderColor *ptr_dst_pixel = ptr_dst_row_start;

        for (int dst_x = dst_x1; dst_x <= dst_x2; dst_x++)
            SWRender_AlphaBlendRGBA8888(*ptr_src_pixel++, ptr_dst_pixel++);
        
        ptr_src_row_start = (SWRenderColor *)((uint8_t *)ptr_src_row_start + linesize_src);
        ptr_dst_row_start = (SWRenderColor *)((uint8_t *)ptr_dst_row_start + linesize_dst);
    }
}

void SWRender_CopyBufferScaled(const SWRenderBuffer *src, const SWRenderBoundI *src_crop, SWRenderBuffer *dst, const SWRenderBoundI *dst_pos)
{
    if (CheckBuffer(src) || CheckBuffer(dst))
        return;

    bool horz_flipped = false;
    bool vert_flipped = false;

    // destination
    int dst_x1, dst_y1, dst_x2, dst_y2;
    int dst_w = dst->w;
    int dst_h = dst->h;

    if (dst_pos)
    {
        dst_x1 = dst_pos->x1;
        dst_y1 = dst_pos->y1;
        dst_x2 = dst_pos->x2;
        dst_y2 = dst_pos->y2;
    }
    else
    {
        dst_x1 = dst_y1 = 0;
        dst_x2 = dst_w - 1;
        dst_y2 = dst_h - 1;
    }

    if (dst_x1 > dst_x2)
    {
        horz_flipped = true;

        int temp = dst_x1;
        dst_x1 = dst_x2;
        dst_x2 = temp;
    }
    if (dst_y1 > dst_y2)
    {
        vert_flipped = true;

        int temp = dst_y1;
        dst_y1 = dst_y2;
        dst_y2 = temp;
    }

    // draw
    int draw_x1 = dst_x1;
    int draw_y1 = dst_y1;
    int draw_x2 = dst_x2;
    int draw_y2 = dst_y2;

    if ((draw_x2 < 0) || (draw_y2 < 0))
        return;
    if ((draw_x1 >= dst_w) || (draw_y1 >= dst_h))
        return;

    if (draw_x1 < 0) draw_x1 = 0;
    if (draw_y1 < 0) draw_y1 = 0;
    if (draw_x2 >= dst_w) draw_x2 = dst_w - 1;
    if (draw_y2 >= dst_h) draw_y2 = dst_h - 1;

    // source
    int src_x1, src_y1, src_x2, src_y2;
    int src_w = src->w;
    int src_h = src->h;

    if (src_crop)
    {
        src_x1 = src_crop->x1;
        src_y1 = src_crop->y1;
        src_x2 = src_crop->x2;
        src_y2 = src_crop->y2;
    }
    else
    {
        src_x1 = src_y1 = 0;
        src_x2 = src_w - 1;
        src_y2 = src_h - 1;
    }

    // draw
    int dst_x1x2_dist = dst_x2 - dst_x1;
    int dst_y1y2_dist = dst_y2 - dst_y1;

    int linesize_src = src->linesize;
    int linesize_dst = dst->linesize;

    SWRenderColor *ptr_dst_row_start = (SWRenderColor *)((uint8_t *)dst->data + (linesize_dst * draw_y1)) + draw_x1;

    for (int y = draw_y1; y <= draw_y2; y++)
    {
        float progress_y = (y - dst_y1) / (float)dst_y1y2_dist;
        if (vert_flipped) progress_y = 1.0F - progress_y;

        int src_y = (int)((src_y1 + ((src_y2 - src_y1) * progress_y)) + 0.5F);
        if (src_y < 0) src_y = 0;
        if (src_y >= src_h) src_y = src_h - 1;

        SWRenderColor *ptr_src_row_start = (SWRenderColor *)((uint8_t *)src->data + (linesize_src * src_y));
        SWRenderColor *ptr_dst_pixel = ptr_dst_row_start;

        for (int x = draw_x1; x <= draw_x2; x++)
        {
            float progress_x = (x - dst_x1) / (float)dst_x1x2_dist;
            if (horz_flipped) progress_x = 1.0F - progress_x;

            int src_x = (int)((src_x1 + ((src_x2 - src_x1) * progress_x)) + 0.5F);
            if (src_x < 0) src_x = 0;
            if (src_x >= src_w) src_x = src_w - 1;

            SWRender_AlphaBlendRGBA8888(ptr_src_row_start[src_x], ptr_dst_pixel);

            ptr_dst_pixel++;
        }

        ptr_dst_row_start = (SWRenderColor *)((uint8_t *)ptr_dst_row_start + linesize_dst);
    }
}

#pragma endregion
