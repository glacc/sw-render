#include <stdint.h>

#define __LOG_FILE__ "main.c"
#include "MacroLog.h"

#include "SWRender.h"
#include "SWRenderExtImage.h"
#include "FTEssentials.h"
#include "FTRender.h"

static const char *fontpath = "./MiSans_Regular.ttf";

int TestSWRender(void)
{
    int ret;

    const int size = 512;
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
