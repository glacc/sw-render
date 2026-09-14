#include "SWRender.h"

#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#define __LOG_FILE__ "SWRenderExt.c"
#include "MacroLog.h"

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

static int CheckBuffer(SWRenderBuffer *buffer)
{
    if (!buffer)
        return -1;

    if (!buffer->data)
        return -1;

    return 0;
}

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

    if (x2 < x1)
    {
        int temp = x1;
        x1 = x2;
        x2 = temp;
    }
    if (y2 < y1)
    {
        int temp = y1;
        y1 = y2;
        y2 = temp;
    }

    if ((x2 < 0) || (x1 >= buf_w) || (y2 < 0) || (y1 >= buf_h))
        return;

    if (x1 < 0) x1 = 0;
    if (y1 < 0) y1 = 0;
    if (x2 >= buf_w) x2 = buf_w - 1;
    if (y2 >= buf_h) y2 = buf_h - 1;

    bool x1x2_same = (x1 == x2);
    bool y1y2_same = (y1 == y2);

    int linesize = buffer->linesize;
    SWRenderColor *ptr_row_start = (SWRenderColor *)((uint8_t *)buffer->data + (y1 * linesize)) + x1;
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

            ptr_row_start = (SWRenderColor *)((uint8_t *)ptr_row_start + linesize);
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

        ptr_row_start = (SWRenderColor *)((uint8_t *)ptr_row_start + linesize);
    }
}

static bool CheckIntersectionHorz();
static bool CheckIntersectionVert();
static bool CheckIntersection();
static bool CheckIntersectionRounded();
static bool CheckIntersectionBoundingBox();
static SWRenderVec2F GetIntersectionPointBetweenTwoLines();
static float DistLine();

static void RecursiveBlockedLine(SWRenderBuffer *buffer, const SWRenderBoundI block_bounds, const SWRenderBoundF *line_pos)
{
    int blk_x1 = block_bounds.x1;
    int blk_y1 = block_bounds.y1;
    int blk_x2 = block_bounds.x2;
    int blk_y2 = block_bounds.y2;
    int buf_w = buffer->w;
    int buf_h = buffer->h;

    if ((blk_x1 >= buf_w) || blk_y1 >= buf_h)
        return;

    
}

void SWRender_Line(SWRenderBuffer *buffer, const SWRenderBoundF pos, float thickness)
{
    if (CheckBuffer(buffer))
        return;

    if (thickness <= 0)
        return;

    // check
    float chk_x1 = pos.x1;
    float chk_y1 = pos.y1;
    float chk_x2 = pos.x2;
    float chk_y2 = pos.y2;
    int buf_w = buffer->w;
    int buf_h = buffer->h;

    if (chk_x2 > chk_x1)
    {
        float temp = chk_x1;
        chk_x1 = chk_x2;
        chk_x2 = temp;
    }
    if (chk_y2 > chk_y1)
    {
        float temp = chk_y1;
        chk_y1 = chk_y2;
        chk_y2 = temp;
    }

    float chk_lt_bound = -thickness;
    float chk_r_bound = buf_w + thickness;
    float chk_b_bound = buf_h + thickness;

    if ((chk_x1 < chk_lt_bound) || (chk_x2 > chk_r_bound))
        return;
    if ((chk_y1 < chk_lt_bound) || (chk_y2 > chk_b_bound))
        return;

}

void SWRender_CopyBuffer(SWRenderBuffer *src, const SWRenderBoundI *src_crop, SWRenderBuffer *dst, const SWRenderVec2I dst_pos_up_left)
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
    if (src_x1 > src_x2)
    {
        int temp = src_x1;
        src_x1 = src_x2;
        src_x2 = temp;
    }
    if (src_y1 > src_y2)
    {
        int temp = src_y1;
        src_y1 = src_y2;
        src_y2 = temp;
    }

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

void SWRender_CopyBufferScaled(SWRenderBuffer *src, const SWRenderBoundI *src_crop, SWRenderBuffer *dst, const SWRenderBoundI *dst_pos)
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
