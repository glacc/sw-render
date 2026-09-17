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

#include <stdint.h>
#include <math.h>

#define __LOG_FILE__ "main.c"
#include "MacroLog.h"

#include "SWRender/SWRender.h"
#include "SWRender/SWRenderExtImage.h"
#include "SWRender/FTEssentials.h"
#include "SWRender/FTRender.h"

static const char *fontpath = "./MiSans_Regular.ttf";

const SWRenderColor color_transition_table[] =
{
    { 0xFF, 0x00, 0x00, 0xFF },
    { 0xFF, 0xFF, 0x00, 0xFF },
    { 0x00, 0xFF, 0x00, 0xFF },
    { 0x00, 0xFF, 0xFF, 0xFF },
    { 0x00, 0x00, 0xFF, 0xFF },
    { 0xFF, 0x00, 0xFF, 0xFF },
    { 0xFF, 0x00, 0x00, 0xFF },
};
const int color_transition_table_length = (sizeof(color_transition_table) / sizeof(color_transition_table[0]));

SWRenderColor GetTransitionColor(float alpha, float progress)
{
    float progress_total = (float)(color_transition_table_length - 1) * progress;
    float progress_index = progress_total - floorf(progress_total);

    int index = (int)progress_total;
    if (index < 0)
        index = 0;
    if (index >= color_transition_table_length - 1)
        index = color_transition_table_length - 2;

    const SWRenderColor *color_curr = &color_transition_table[index];
    const SWRenderColor *color_next = &color_transition_table[index + 1];

    float r = (float)color_curr->r + ((float)(color_next->r - color_curr->r) * progress_index);
    float g = (float)color_curr->g + ((float)(color_next->g - color_curr->g) * progress_index);
    float b = (float)color_curr->b + ((float)(color_next->b - color_curr->b) * progress_index);
    float a = (float)color_curr->a + ((float)(color_next->a - color_curr->a) * progress_index);
    a *= alpha;

    return (SWRenderColor){ (uint8_t)r, (uint8_t)g, (uint8_t)b, (uint8_t)a };
}

int TestSWRender(void)
{
    int ret;

    const int size = 512;
    const int half_size = size / 2;
    const int color_size = 192;

    FTEssentials_State ft_state;
    SWRenderBuffer buffer_output = { size, size, NULL, 0 };
    SWRenderBuffer buffer_scaled = { size, size, NULL, 0 };
    SWRenderBuffer buffer_image = { 0, 0, NULL, 0 };

    if (!FTEssentials_InitState(&ft_state, "./MiSans_Regular.ttf"))
    {
        fprintf(stderr, LOCATION_PREFIX_STR "failed to open font \'%s\'.\n", fontpath);
        ret = -1;
        goto end;
    }

    if (SWRender_BufferAlloc(&buffer_output) < 0)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "buffer_output mem alloc failed.\n");
        ret = -1;
        goto end;
    }

    if (SWRender_BufferAlloc(&buffer_scaled) < 0)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "buffer_output mem alloc failed.\n");
        ret = -1;
        goto end;
    }

    // output
    // alpha background
    SWRender_FillRect(&buffer_output, (SWRenderBoundI){ 0, 0, size - 1, size - 1 }, (SWRenderColor){ 0x00, 0x00, 0x00, 0x00 }, true);

    // blit image?
    SWRender_LoadImageToBuffer("./test_image.jpg", &buffer_image);
    SWRender_CopyBuffer(&buffer_image, &(SWRenderBoundI){ 0, 0, buffer_image.w * 2 / 3, buffer_image.h }, &buffer_output, (SWRenderVec2I){ (-buffer_image.w / 4), 0 });

    // white background
    SWRender_FillRect(&buffer_output, (SWRenderBoundI){ 0, 0, (size * 3 / 4) - 1, (size * 3 / 4) - 1 }, (SWRenderColor){ 0xFF, 0xFF, 0xFF, 0xFF }, false);

    // colors
    SWRender_FillRect(&buffer_output, (SWRenderBoundI){ 16, 16, 16 + color_size, 16 + color_size }, (SWRenderColor){ 0xFF, 0x00, 0x00, 0x80 }, false);
    SWRender_FillRect(&buffer_output, (SWRenderBoundI){ 32, 32, 32 + color_size, 32 + color_size }, (SWRenderColor){ 0x00, 0xFF, 0x00, 0x80 }, false);
    SWRender_FillRect(&buffer_output, (SWRenderBoundI){ 48, 48, 48 + color_size, 48 + color_size }, (SWRenderColor){ 0x00, 0x00, 0xFF, 0x80 }, false);

    // test text
    FTEssentials_SetSize96DPI(&ft_state, INT_TO_F26DOT6(18));

    FTRender_RenderStrWithAlign(&ft_state, &buffer_output, INT_TO_F26DOT6(56), INT_TO_F26DOT6(24), "Hello, SWRender by Glacc.\nqwq", -1, -1, (SWRenderColor){ 0x00, 0x00, 0x00, 0x40 });

    FTRender_RenderStrWithAlign(&ft_state, &buffer_output, INT_TO_F26DOT6(24), INT_TO_F26DOT6(256), "RED", -1, -1, (SWRenderColor){ 0xFF, 0x00, 0x00, 0xFF });
    FTRender_RenderStrWithAlign(&ft_state, &buffer_output, INT_TO_F26DOT6(24), INT_TO_F26DOT6(280), "GREEN", -1, -1, (SWRenderColor){ 0x00, 0xFF, 0x00, 0xFF });
    FTRender_RenderStrWithAlign(&ft_state, &buffer_output, INT_TO_F26DOT6(24), INT_TO_F26DOT6(304), "BLUE", -1, -1, (SWRenderColor){ 0x00, 0x00, 0xFF, 0xFF });

    FTRender_RenderStrWithAlign(&ft_state, &buffer_output, INT_TO_F26DOT6(192), INT_TO_F26DOT6(420), "Alpha Blend", -1, -1, (SWRenderColor){ 0x00, 0x7A, 0xCC, 0x80 });

    FTRender_RenderStrWithAlign(&ft_state, &buffer_output, INT_TO_F26DOT6(420), INT_TO_F26DOT6(420), "喵！", -1, -1, (SWRenderColor){ 0xFF, 0xFF, 0xFF, 0xFF });

    // lines
    const int line_count = 24;
    const float line_start_dist = 8.0F;
    const float line_end_dist = 160.0F;
    const float line_thickness = 5.0F;
    const float bg_ring_thickness = 5.0F;
    const float bg_radius = line_end_dist + bg_ring_thickness + line_thickness + line_thickness;
    const float bg_rect_half_size = bg_radius + bg_ring_thickness + bg_ring_thickness;
    const float bg_rect_pos_ul = half_size - bg_rect_half_size;
    const float bg_rect_pos_dr = half_size + bg_rect_half_size;
    const float bg_rect_corner_radius = 10.0F;

    const float angle_offset = -M_PI_2f;

    SWRender_FillRectRounded(&buffer_output, (SWRenderBoundF){ bg_rect_pos_ul, bg_rect_pos_ul, bg_rect_pos_dr, bg_rect_pos_dr }, bg_rect_corner_radius, (SWRenderColor){ 0x00, 0x00, 0x00, 0x40 });
    SWRender_FillCircle(&buffer_output, (SWRenderVec2F){ half_size , half_size }, bg_radius, NULL, (SWRenderColor){ 0x00, 0x00, 0x00, 0x40 });
    SWRender_FillRing(&buffer_output, (SWRenderVec2F){ half_size, half_size }, bg_radius, bg_ring_thickness, NULL, (SWRenderColor){ 0x00, 0x00, 0x00, 0x40 });

    for (int i = 0; i < line_count; i++)
    {
        float progress = (float)i * (1.0F / (float)line_count);
        float angle = (progress * 2.0F * M_PI) + angle_offset;

        float x1 = half_size + (cosf(angle) * line_start_dist);
        float y1 = half_size + (sinf(angle) * line_start_dist);
        float x2 = half_size + (cosf(angle) * line_end_dist);
        float y2 = half_size + (sinf(angle) * line_end_dist);

        SWRender_Line(&buffer_output, &(SWRenderBoundF){ x1, y1, x2, y2 }, GetTransitionColor((0x80 / 255.0F), progress), line_thickness);
    }

    // scaled
    // large
    SWRender_CopyBufferScaled(&buffer_output, NULL, &buffer_scaled, &(SWRenderBoundI){ 0, 0, (size * 2) - 1, (size * 2) - 1});

    // small
    SWRender_CopyBufferScaled(&buffer_output, NULL, &buffer_scaled, &(SWRenderBoundI){ 0, 0, (size / 2) - 1, (size / 2) - 1});
    SWRender_CopyBufferScaled(&buffer_output, NULL, &buffer_scaled, &(SWRenderBoundI){ size / 2, 0, size - 1, (size / 2) - 1});
    SWRender_CopyBufferScaled(&buffer_output, NULL, &buffer_scaled, &(SWRenderBoundI){ 0, size / 2, (size / 2) - 1, size - 1});
    SWRender_CopyBufferScaled(&buffer_output, NULL, &buffer_scaled, &(SWRenderBoundI){ size / 2, size / 2, size - 1, size - 1});

    // save
    SWRender_SaveImageFromBuffer(&buffer_output, "./test-sw-render.png", AV_CODEC_ID_PNG);
    SWRender_SaveImageFromBuffer(&buffer_scaled, "./test-sw-render-scaled.png", AV_CODEC_ID_PNG);

end:
    SWRender_BufferFree(&buffer_image);

    SWRender_BufferFree(&buffer_scaled);
    SWRender_BufferFree(&buffer_output);

    FTEssentials_Done(&ft_state);

    return ret;
}

int main(int argc, char *argv[])
{
    TestSWRender();

    return 0;
}
