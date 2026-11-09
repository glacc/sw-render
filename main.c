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
#include "SWRender/FTEssentialsWordWrap.h"
#include "SWRender/FTRender.h"
#include "SWRender/FTRenderWordWrap.h"

const char *test_str_1 = "Once upon a time a little girl tried to make a living by selling matches in the street. It was New Year's Eve and the snowed streets were deserted. From brightly lit windows came the tinkle of laughter and the sound of singing. People were getting ready to bring in the new year. But the poor little match seller sat sadly beside the fountain. Her ragged dress and worn shawl did not keep out the cold and she tried to keep her bare feet from touching the frozen ground. She hadn't sold one box of matches all day and she was frightened to go home, for her father would certainly be angry. It wouldn't be much warmer anyway, in the draughty attic that was her home. The little girl's fingers were stiff with cold. If only she could light a match! But what would her father say at such a waste. Falteringly she took out a match and lit it. What a nice warm flame! The little match seller cupped her hand over it, and as she did so, she magically saw in its light a big brightly burning stove. She held out her hands to the heat, but just then the match went out and the vision faded. The night seemed blacker than before and it was getting colder. A shiver ran through the little girl's thin body. After hesitating for a long time, she struck another match on the wall, and this time, the glimmer turned the wall into a great sheet of crystal. Beyond that stood a fine table laden with food and lit by a candlestick. Holding out her arms towards the plates, the little match seller seemed to pass through the glass, but then the match went out and the magic faded. Poor thing: in just a few seconds she had caught a glimpse of everything that life had denied her: warmth and good things to eat. Her eyes filled with tears and she lifted her gaze to the lit windows, praying that she too might know a little of such happiness. She lit the third match and an even more wonderful thing happened. There stood a Christmas tree hung with hundreds of candles, glittering with tinsel and coloured balls. \"Oh, how lovely!\" exclaimed the little match seller, holding up the match. Then, the match burned her finger and flickered out. The light from the Christmas candles rose higher and higher. Then one of the lights fell, leaving a trail behind it. \"Someone is dying,\" murmured the little girl, as she remembered her beloved Granny who used to say: \"When a star falls, a heart stops beating!\" Scarcely aware of what she was doing, the little match seller lit another match. This time, she saw her grandmother. A cold day dawned and a pale sun shone on the fountain and the icy road. Close by lay the lifeless body of a little girl surrounded by spent matches. \"Poor little thing!\" Exclaimed the passersby. She was trying to keep warm.";
const char *test_str_2 = "冰灯是流行于中国北方的一种古老的民间艺术形式。因为独特的地域优势，黑龙江可以说是制作冰灯最早的地方。传说在很早以前，每到冬季的夜晚，在松嫩平原上，人们总会看到三五成群的农夫和渔民在悠然自得地喂马和捕鱼，他们所使用的照明工具就是用冰做成的灯笼。这便是最早的冰灯。当时制作冰灯的工艺也很简单，把水放进木桶里冻成冰坨，凿出空心，放个油灯在里面，用以照明，冰罩挡住了凛冽的寒风，黑夜里便有了不灭的灯盏，冰灯成了人们生活中不可缺少的帮手。后来，每逢新春佳节和上元之夜，人们又把它加以装饰，而成为供人观赏的独特的艺术表现形式。清代《黑龙江外纪》里对此有过详细的记载：“上元，城中张灯五夜，村落妇女来观剧者，车声彻夜不绝。有镂五六尺冰为寿星灯者，中燃双炬，望之如水晶人。”其实，冰灯在南方一些地方也相继出现过。乾隆、嘉庆年间，四川诗人张问陶曾写过一首专门描写冰灯的诗，题名就叫《冰灯》，诗云：“黑夜有炎凉，冰灯吐焰长。照来消热念，凿处漏寒光。影湿星沉水，神清月里霜。三冬足文史，底用探萤囊。”南京诗人金德荣在被谪戍新疆巴里坤时，在其古风长诗《巴里坤冰灯歌》中也咏叹道：“雪山高与天山接，上有万古不化雪。朔风一夜结作冰，裁雪妙手搏为冰。以矾入冰冰不化，以烛照冰光四射。五里之内尽通明，半月能教天不夜。元夕月轮照碧空，大千人入水晶宫。”哈尔滨是中国冰雪艺术的摇篮，哈尔滨冰灯驰名中外，饮誉华夏。哈尔滨大规模有组织地制作和展出冰灯始于1963年，人们利用盆、桶等简单模具自然冷冻了千余盏冰灯和数十个冰花，于元宵佳节在兆麟公园展出，轰动全城，形成了万人空巷看冰灯的盛大场面。至今许多老哈尔滨人回想起来仍然记忆犹新，感慨万千。这也是我国第一个有组织、有领导的冰灯游园会。当时就有人即兴作词，来形容这“万人空巷，盛极一时”的今古奇观：“灯节，灯节，玉树冰灯明月。人山人海兴浓，园北园南烛红。红烛，红烛，普照万民同乐。”冰灯是黑土地的特产，是黑龙江人的骄傲。从盆制冰景到一年一度大规模的冰灯游园会，哈尔滨冰灯艺术日趋成熟，它的影响和辐射早已使其驰名世界，风靡海内外。1985年，勤劳智慧的冰城人民进一步挖掘冰雪热能，开发冰雪资源，以蜚声中外的冰灯游园会为中心，推出了以冰雪艺术、冰雪体育、冰雪文化、冰雪旅游、冰雪经贸为内容的哈尔滨冰雪节，把让人畏惧的冰雪变成了宝贵的自然资源，给冰城之冬增添了盎然的春意。以后每年的1月5日，便成为哈尔滨人民特有的地方性传统节日，北方人改变了足不出户的“猫冬”习惯，开始参加各种冰雪活动，哈尔滨之冬不再寂寞，哈尔滨的冬天热了起来。";

static const char *fontpath = "./MiSans_Regular.ttf";
static const char *testimg_path = "./test_image.jpg";

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

static int TestWordWrap()
{
    int ret = 0;

    FTEssentials_State ft_state = { };
    FTEssentialsWordWrap_LineBuffer line_buffer = { };
    SWRenderBuffer buffer = { 2048, 2048, NULL, 0 };

    ft_state.linespace_multiplier = 1.0F;

    if (!FTEssentials_InitState(&ft_state, fontpath))
    {
        fprintf(stderr, LOCATION_PREFIX_STR "failed to open font \'%s\'.\n", fontpath);
        ret = -1;
        goto end;
    }

    if (!FTEssentialsWordWrap_LineBufferInit(&line_buffer))
    {
        fprintf(stderr, LOCATION_PREFIX_STR "line_buffer mem alloc failed.\n");
        ret = -1;
        goto end;
    }

    if (!SWRender_BufferAlloc(&buffer))
    {
        fprintf(stderr, LOCATION_PREFIX_STR "buffer mem alloc failed.\n");
        ret = -1;
        goto end;
    }

    FTEssentials_SetSize96DPI(&ft_state, INT_TO_F26DOT6(16));

    SWRenderBoundI buffer_bounds;
    SWRender_BufferGetBounds(&buffer_bounds, &buffer);

    SWRender_FillRect(&buffer, &buffer_bounds, (SWRenderColor){ 255, 255, 255, 255 }, true);

    //FTRenderWordWrap_RenderStr(&ft_state, &line_buffer, &buffer, INT_TO_F26DOT6(16), INT_TO_F26DOT6(64), INT_TO_F26DOT6(1024 - 32), test_str_2, (SWRenderColor){ 0, 0, 0, 255 });
    //FTRenderWordWrap_RenderStr(&ft_state, &line_buffer, &buffer, INT_TO_F26DOT6(1024 + 16), INT_TO_F26DOT6(64), INT_TO_F26DOT6(1024 - 32), test_str_2, (SWRenderColor){ 0, 0, 0, 255 });

    SWRenderRectF str_rect =
    {
        16,
        16,
        1024 - 32,
        2048 - 32,
    };
    FTRenderWordWrap_RenderStrWithAlign(&ft_state, &line_buffer, &buffer, &str_rect, test_str_1, 0, 0, (SWRenderColor){ 0, 0, 0, 255 });
    str_rect.x += 1024;
    FTRenderWordWrap_RenderStrWithAlign(&ft_state, &line_buffer, &buffer, &str_rect, test_str_2, 0, 0, (SWRenderColor){ 0, 0, 0, 255 });

    SWRender_SaveImageFromBuffer(&buffer, "./test-word-wrap.png", AV_CODEC_ID_PNG);
    
end:
    FTEssentialsWordWrap_LineBufferCleanup(&line_buffer);
    FTEssentials_Done(&ft_state);
    SWRender_BufferFree(&buffer);

    return ret;
}

static int TestSWRender(void)
{
    int ret;

    const int size = 512;
    const int half_size = size / 2;
    const int color_size = 192;

    FTEssentials_State ft_state;
    SWRenderBuffer buffer_output = { size, size, NULL, 0 };
    SWRenderBuffer buffer_scaled = { size, size, NULL, 0 };
    SWRenderBuffer buffer_image = { 0, 0, NULL, 0 };

    if (!FTEssentials_InitState(&ft_state, fontpath))
    {
        fprintf(stderr, LOCATION_PREFIX_STR "failed to open font \'%s\'.\n", fontpath);
        ret = -1;
        goto end;
    }

    if (!SWRender_BufferAlloc(&buffer_output))
    {
        fprintf(stderr, LOCATION_PREFIX_STR "buffer_output mem alloc failed.\n");
        ret = -1;
        goto end;
    }

    if (!SWRender_BufferAlloc(&buffer_scaled))
    {
        fprintf(stderr, LOCATION_PREFIX_STR "buffer_output mem alloc failed.\n");
        ret = -1;
        goto end;
    }

    // output
    // alpha background
    SWRender_FillRect(&buffer_output, &(SWRenderBoundI){ 0, 0, size - 1, size - 1 }, (SWRenderColor){ 0x00, 0x00, 0x00, 0x00 }, true);

    // blit image?
    SWRender_LoadImageFromFileToBuffer(testimg_path, &buffer_image);
    SWRender_CopyBuffer(&buffer_image, &(SWRenderBoundI){ 0, 0, buffer_image.w * 2 / 3, buffer_image.h }, &buffer_output, (SWRenderVec2I){ (-buffer_image.w / 4), 0 });

    // white background
    SWRender_FillRect(&buffer_output, &(SWRenderBoundI){ 0, 0, (size * 3 / 4) - 1, (size * 3 / 4) - 1 }, (SWRenderColor){ 0xFF, 0xFF, 0xFF, 0xFF }, false);

    // colors
    SWRender_FillRect(&buffer_output, &(SWRenderBoundI){ 16, 16, 16 + color_size, 16 + color_size }, (SWRenderColor){ 0xFF, 0x00, 0x00, 0x80 }, false);
    SWRender_FillRect(&buffer_output, &(SWRenderBoundI){ 32, 32, 32 + color_size, 32 + color_size }, (SWRenderColor){ 0x00, 0xFF, 0x00, 0x80 }, false);
    SWRender_FillRect(&buffer_output, &(SWRenderBoundI){ 48, 48, 48 + color_size, 48 + color_size }, (SWRenderColor){ 0x00, 0x00, 0xFF, 0x80 }, false);

    // test text
    FTEssentials_SetSize96DPI(&ft_state, INT_TO_F26DOT6(18));

    FTRender_RenderStrWithAlign(&ft_state, &buffer_output, INT_TO_F26DOT6(56), INT_TO_F26DOT6(24), "Hello, SWRender by Glacc.\nqwq", SIZE_MAX, -1, -1, (SWRenderColor){ 0x00, 0x00, 0x00, 0x40 });

    FTRender_RenderStrWithAlign(&ft_state, &buffer_output, INT_TO_F26DOT6(24), INT_TO_F26DOT6(256), "RED", SIZE_MAX, -1, -1, (SWRenderColor){ 0xFF, 0x00, 0x00, 0xFF });
    FTRender_RenderStrWithAlign(&ft_state, &buffer_output, INT_TO_F26DOT6(24), INT_TO_F26DOT6(280), "GREEN", SIZE_MAX, -1, -1, (SWRenderColor){ 0x00, 0xFF, 0x00, 0xFF });
    FTRender_RenderStrWithAlign(&ft_state, &buffer_output, INT_TO_F26DOT6(24), INT_TO_F26DOT6(304), "BLUE", SIZE_MAX, -1, -1, (SWRenderColor){ 0x00, 0x00, 0xFF, 0xFF });

    FTRender_RenderStrWithAlign(&ft_state, &buffer_output, INT_TO_F26DOT6(192), INT_TO_F26DOT6(420), "Alpha Blend", SIZE_MAX, -1, -1, (SWRenderColor){ 0x00, 0x7A, 0xCC, 0x80 });

    FTRender_RenderStrWithAlign(&ft_state, &buffer_output, INT_TO_F26DOT6(420), INT_TO_F26DOT6(420), "喵！", SIZE_MAX, -1, -1, (SWRenderColor){ 0xFF, 0xFF, 0xFF, 0xFF });

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

    SWRender_FillRectRounded(&buffer_output, &(SWRenderBoundF){ bg_rect_pos_ul, bg_rect_pos_ul, bg_rect_pos_dr, bg_rect_pos_dr }, bg_rect_corner_radius, (SWRenderColor){ 0x00, 0x00, 0x00, 0x40 });
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
    TestWordWrap();

    return 0;
}
