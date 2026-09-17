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

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <qoi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/limits.h>

#define __LOG_FILE__ "ImageSource.c"
#include "MacroLog.h"

#include "SWRender/SWRender.h"
#include "SWRender/SWRenderExtImage.h"
#include "SWRender/FTEssentials.h"
#include "SWRender/FTRender.h"

static const char *images_to_test[] = {
    "./test_image.png",
    "./test_image.jpg",
    NULL,
};

static const char *save_file_ext = ".png";
static const enum AVCodecID encode_codec_id = AV_CODEC_ID_PNG;

static const char *fontpath = "./MiSans_Regular.ttf";

int ConvertFramePixFmtFitToEncoder(AVFrame *src, AVFrame *dst, const AVCodec *encoder)
{
    int ret = 0;

    const enum AVPixelFormat *pix_fmt_list = NULL;
    int pix_fmt_cnt;
    avcodec_get_supported_config(NULL, encoder, AV_CODEC_CONFIG_PIX_FORMAT, 0, (const void **)&pix_fmt_list, &pix_fmt_cnt);

    enum AVPixelFormat *pix_fmt_none_term_list = aligned_alloc(sizeof(enum AVPixelFormat), sizeof(enum AVPixelFormat) * (pix_fmt_cnt + 1));
    if (!pix_fmt_none_term_list)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "pix_fmt_none_term_list memory allocation failed.\n");
        ret = -1;
        goto end;
    }
    memcpy(pix_fmt_none_term_list, pix_fmt_list, sizeof(enum AVPixelFormat) * pix_fmt_cnt);
    pix_fmt_none_term_list[pix_fmt_cnt] = AV_PIX_FMT_NONE;

    enum AVPixelFormat dst_pix_fmt = avcodec_find_best_pix_fmt_of_list(pix_fmt_none_term_list, src->format, true, NULL);
    if (dst_pix_fmt == -1)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avcodec_find_best_pix_fmt_of_list failed to find dst pixel format.\n");
        ret = -1;
        goto end;
    }

    av_frame_unref(dst);
    dst->width  = src->width;
    dst->height = src->height;
    dst->format = dst_pix_fmt;
    if ((ret = av_frame_get_buffer(dst, 0)) < 0)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "av_frame_get_buffer failed.\n");
        goto end;
    }

    struct SwsContext *sws_ctx = sws_getContext(src->width, src->height, src->format, dst->width, dst->height, dst->format, SWS_BICUBIC, NULL, NULL, NULL);
    if (!sws_ctx)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "sws_alloc_context failed.\n");
        ret = -1;
        goto end;
    }
    
    sws_scale(sws_ctx, (const uint8_t *const *)src->data, src->linesize, 0, src->height, (uint8_t *const *)dst->data, dst->linesize);

end:
    if (pix_fmt_none_term_list) free(pix_fmt_none_term_list);
    sws_free_context(&sws_ctx);

    return ret;
}

void EncodeImageAndSaveToFile(const char *filepath, AVFrame *src_frame, enum AVCodecID codec_id)
{
    int ret;

    FILE *file = fopen(filepath, "wb");
    if (!file)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "failed to open file \'%s\'\n", filepath);
        return;
    }

    const AVCodec *codec = avcodec_find_encoder(codec_id);
    if (!codec)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avcodec_find_encoder failed.\n");
        goto end;
    }

    AVFrame *dst_frame = av_frame_alloc();
    if (!dst_frame)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "av_frame_alloc failed.\n");
        goto end;
    }

    if (ConvertFramePixFmtFitToEncoder(src_frame, dst_frame, codec))
    {
        fprintf(stderr, LOCATION_PREFIX_STR "frame format convertion failed.\n");
        goto end;
    }

    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    if (!codec_ctx)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avcodec_alloc_context3 failed.\n");
        goto end;
    }

    codec_ctx->width = src_frame->width;
    codec_ctx->height = src_frame->height;
    codec_ctx->time_base = (AVRational){ 1, 25 };
    codec_ctx->framerate = (AVRational){ 25, 1 };
    codec_ctx->pix_fmt = dst_frame->format;

    if (avcodec_open2(codec_ctx, codec, NULL) < 0)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avcodec_open2 failed.\n");
        goto end;
    }

    AVPacket *pkt = av_packet_alloc();
    if (!pkt)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "av_packet_alloc failed.\n");
        goto end;
    }

    if ((ret = avcodec_send_frame(codec_ctx, dst_frame)) < 0)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avcodec_send_frame failed.\n");
        goto end;
    }

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(codec_ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        else if (ret < 0)
        {
            fprintf(stderr, LOCATION_PREFIX_STR "avcodec_receive_packet failed.\n");
            break;
        }

        fwrite(pkt->data, 1, pkt->size, file);

        av_packet_unref(pkt);
    }

end:
    av_frame_free(&dst_frame);
    av_packet_free(&pkt);
    avcodec_free_context(&codec_ctx);
    fclose(file);
}

void SaveAVFrameToQoiFile(AVFrame *frame, const char *filepath)
{
    // remove gap
    uint32_t *qoi_src_data = aligned_alloc(64, sizeof(uint32_t) * frame->width * frame->height);
    if (!qoi_src_data)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "qoi_src_data memory allocation failed.\n");
        goto end;
    }

    uint8_t *ptr_sws_dst = frame->data[0];
    uint32_t *ptr_qoi_src = qoi_src_data;
    for (int y = 0; y < frame->height; y++)
    {
        memcpy(ptr_qoi_src, ptr_sws_dst, frame->width * 4);

        ptr_sws_dst += frame->linesize[0];
        ptr_qoi_src += frame->width;
    }

    // save
    qoi_desc dst_qoi_desc = { frame->width, frame->height, 4, QOI_SRGB };
    qoi_write(filepath, qoi_src_data, &dst_qoi_desc);

    printf(LOCATION_PREFIX_STR "frame saved to qoi file \'%s\'\n", filepath);

end:
    if (qoi_src_data) free(qoi_src_data);
}

void SavePixelsFromFirstFrameOfVideoFileToFileInSameNameWithNewExt(const char *filepath)
{
    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *codec_ctx = NULL;
    AVPacket *pkt = NULL;
    AVFrame *frame = NULL;
    int ret;

    fmt_ctx = avformat_alloc_context();
    if (!fmt_ctx)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avformat_alloc_context failed.\n");
        goto end;
    }

    if (avformat_open_input(&fmt_ctx, filepath, NULL, NULL))
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avformat_open_input failed.\n");
        goto end;
    }

    pkt = av_packet_alloc();
    if (!pkt)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "av_packet_alloc failed.\n");
        goto end;
    }

    frame = av_frame_alloc();
    if (!frame)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "av_frame_alloc failed.\n");
        goto end;
    }

    int first_video_stream_index = -1;
    AVCodecParameters *first_video_stream_codec_para = NULL;
    const AVCodec *first_video_stream_codec = NULL;

    avformat_find_stream_info(fmt_ctx, NULL);
    for (int i = 0; i < fmt_ctx->nb_streams; i++)
    {
        AVStream *stream_current = fmt_ctx->streams[i];
        printf(LOCATION_PREFIX_STR "stream index %d, id %d, frames %ld\n", stream_current->index, stream_current->id, stream_current->nb_frames);

        AVCodecParameters *codec_para = stream_current->codecpar;
        const AVCodec *codec = avcodec_find_decoder(codec_para->codec_id);
        if (!codec)
        {
            fprintf(stderr, LOCATION_PREFIX_STR "avcodec_find_decoder failed for stream at index %d.\n", stream_current->index);
            continue;
        }

        if (codec_para->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            if (first_video_stream_index < 0)
            {
                first_video_stream_index = stream_current->index;
                first_video_stream_codec = codec;
                first_video_stream_codec_para = codec_para;

                printf(LOCATION_PREFIX_STR "found video stream in size %d x %d\n", codec_para->width, codec_para->height);
            }
        }
    }

    if (first_video_stream_index < 0)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "could not find video stream in file \'%s\'\n", filepath);
        goto end;
    }

    codec_ctx = avcodec_alloc_context3(first_video_stream_codec);
    if (!codec_ctx)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avcodec_alloc_context3 failed.\n");
        goto end;
    }

    if (avcodec_parameters_to_context(codec_ctx, first_video_stream_codec_para) < 0)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avcodec_parameters_to_context failed.\n");
        goto end;
    }
    if (avcodec_open2(codec_ctx, first_video_stream_codec, NULL) < 0)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "avcodec_open2 failed.\n");
        goto end;
    }
    
    bool got_frame = false;
    do
    {
        // [file] --read--> [pkt] --send--> [codec] --receive--> [frame]

        ret = av_read_frame(fmt_ctx, pkt);
        if (ret < 0)
        {
            fprintf(stderr, LOCATION_PREFIX_STR "av_read_frame failed.\n");
            break;
        }

        if (pkt->stream_index == first_video_stream_index)
        {
            ret = avcodec_send_packet(codec_ctx, pkt);
            if (ret < 0)
            {
                fprintf(stderr, LOCATION_PREFIX_STR "avcodec_send_packet failed.\n");
                break;
            }

            do
            {
                ret = avcodec_receive_frame(codec_ctx, frame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    break;
                else if (ret < 0)
                {
                    fprintf(stderr, LOCATION_PREFIX_STR "avcodec_receive_frame failed.\n");
                    break;
                }

                // save content here?
                AVFrame *dst_frame = av_frame_alloc();
                if (!dst_frame)
                {
                    fprintf(stderr, LOCATION_PREFIX_STR "av_frame_alloc failed.\n");
                    break;
                }
                dst_frame->width = frame->width;
                dst_frame->height = frame->height;
                dst_frame->format = AV_PIX_FMT_RGBA;

                struct SwsContext *sws_ctx = sws_getContext(frame->width, frame->height, frame->format, frame->width, frame->height, dst_frame->format, SWS_LANCZOS, NULL, NULL, NULL);
                if (sws_ctx)
                {
                    // dst buffer allocation
                    ret = av_frame_get_buffer(dst_frame, 0);
                    if (ret < 0)
                    {
                        fprintf(stderr, LOCATION_PREFIX_STR "av_frame_get_buffer failed.\n");
                        goto end_scale;
                    }
                    for (int i = 0; i < AV_NUM_DATA_POINTERS; i++)
                    {
                        if (dst_frame->data[i])
                            printf(LOCATION_PREFIX_STR "plane %d, addr 0x%016lX, plane linesize %d\n", i, (uint64_t)dst_frame->data[i], dst_frame->linesize[i]);
                    }
                    
                    // scale (convert)
                    sws_scale(sws_ctx, (const uint8_t *const *)frame->data, frame->linesize, 0, frame->height, dst_frame->data, dst_frame->linesize);
                    
                    // save
                    char filepath_new[NAME_MAX];
                    strcpy(filepath_new, filepath);
                    strcat(filepath_new, save_file_ext);
                    // SaveAVFrameToQoiFile(dst_frame, filepath_new);
                    EncodeImageAndSaveToFile(filepath_new, dst_frame, encode_codec_id);

                end_scale:
                    av_frame_free(&dst_frame);
                    sws_freeContext(sws_ctx);
                }
                else
                    fprintf(stderr, LOCATION_PREFIX_STR "sws_getContext failed.\n");

                got_frame = true;

                break;
            }
            while (!got_frame);
        }

        av_packet_unref(pkt);
    }
    while (!got_frame);

end:
    avcodec_free_context(&codec_ctx);
    av_frame_free(&frame);
    av_packet_free(&pkt);
    avformat_free_context(fmt_ctx);
}

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

    /*
    AVFrame *frame_output = av_frame_alloc();
    if (!frame_output)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "av_frame_alloc failed.\n");
        goto end;
    }

    AVFrame *frame_scaled = av_frame_alloc();
    if (!frame_scaled)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "av_frame_alloc failed.\n");
        goto end;
    }

    frame_output->width = frame_output->height = size;
    frame_output->format = AV_PIX_FMT_RGBA;
    av_frame_get_buffer(frame_output, 0);
    if (ret < 0)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "av_frame_get_buffer failed.\n");
        goto end;
    }
    buffer_output.w = buffer_output.h = size;
    buffer_output.data = (uint32_t *)frame_output->data[0];
    buffer_output.linesize = frame_output->linesize[0];

    frame_scaled->width = frame_scaled->height = size;
    frame_scaled->format = AV_PIX_FMT_RGBA;
    av_frame_get_buffer(frame_scaled, 0);
    if (ret < 0)
    {
        fprintf(stderr, LOCATION_PREFIX_STR "av_frame_get_buffer failed.\n");
        goto end;
    }
    buffer_scaled.w = buffer_scaled.h = size;
    buffer_scaled.data = (uint32_t *)frame_scaled->data[0];
    buffer_scaled.linesize = frame_scaled->linesize[0];
    */

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
    SWRender_FillRect(&buffer_output, (SWRenderRect){ 0, 0, size - 1, size - 1 }, (SWRenderColor){ 0x00, 0x00, 0x00, 0x00 }, true);

    // blit image?
    SWRender_LoadImageToBuffer("test_image.jpg", &buffer_image);
    SWRender_CopyBuffer(&buffer_image, &(SWRenderRect){ 0, 0, buffer_image.w * 2 / 3, buffer_image.h }, &buffer_output, (SWRenderVec2I){ (-buffer_image.w / 4), 0 });

    // white background
    SWRender_FillRect(&buffer_output, (SWRenderRect){ 0, 0, (size * 3 / 4) - 1, (size * 3 / 4) - 1 }, (SWRenderColor){ 0xFF, 0xFF, 0xFF, 0xFF }, false);

    // colors
    SWRender_FillRect(&buffer_output, (SWRenderRect){ 16, 16, 16 + color_size, 16 + color_size }, (SWRenderColor){ 0xFF, 0x00, 0x00, 0x80 }, false);
    SWRender_FillRect(&buffer_output, (SWRenderRect){ 32, 32, 32 + color_size, 32 + color_size }, (SWRenderColor){ 0x00, 0xFF, 0x00, 0x80 }, false);
    SWRender_FillRect(&buffer_output, (SWRenderRect){ 48, 48, 48 + color_size, 48 + color_size }, (SWRenderColor){ 0x00, 0x00, 0xFF, 0x80 }, false);

    // test text
    FTEssentials_SetSize96DPI(&ft_state, INT_TO_F26DOT6(18));

    FTRender_RenderStrWithAlign(&ft_state, &buffer_output, INT_TO_F26DOT6(56), INT_TO_F26DOT6(24), "Hello, SWRender by Glacc.\nqwq", -1, -1, (SWRenderColor){ 0x00, 0x00, 0x00, 0x40 });

    FTRender_RenderStrWithAlign(&ft_state, &buffer_output, INT_TO_F26DOT6(24), INT_TO_F26DOT6(256), "RED", -1, -1, (SWRenderColor){ 0xFF, 0x00, 0x00, 0xFF });
    FTRender_RenderStrWithAlign(&ft_state, &buffer_output, INT_TO_F26DOT6(24), INT_TO_F26DOT6(280), "GREEN", -1, -1, (SWRenderColor){ 0x00, 0xFF, 0x00, 0xFF });
    FTRender_RenderStrWithAlign(&ft_state, &buffer_output, INT_TO_F26DOT6(24), INT_TO_F26DOT6(304), "BLUE", -1, -1, (SWRenderColor){ 0x00, 0x00, 0xFF, 0xFF });

    FTRender_RenderStrWithAlign(&ft_state, &buffer_output, INT_TO_F26DOT6(48), INT_TO_F26DOT6(420), "Alpha Blend", -1, -1, (SWRenderColor){ 0x00, 0x7A, 0xCC, 0x80 });

    FTRender_RenderStrWithAlign(&ft_state, &buffer_output, INT_TO_F26DOT6(420), INT_TO_F26DOT6(420), "喵！", -1, -1, (SWRenderColor){ 0xFF, 0xFF, 0xFF, 0xFF });

    // save
    // EncodeImageAndSaveToFile("./test-sw-render.png", frame_output, AV_CODEC_ID_PNG);
    SWRender_SaveImageFromBuffer(&buffer_output, "./test-sw-render.png", AV_CODEC_ID_PNG);

    // scaled

    // large
    SWRender_CopyBufferScaled(&buffer_output, NULL, &buffer_scaled, &(SWRenderRect){ 0, 0, (size * 2) - 1, (size * 2) - 1});

    // small
    SWRender_CopyBufferScaled(&buffer_output, NULL, &buffer_scaled, &(SWRenderRect){ 0, 0, (size / 2) - 1, (size / 2) - 1});
    SWRender_CopyBufferScaled(&buffer_output, NULL, &buffer_scaled, &(SWRenderRect){ size / 2, 0, size - 1, (size / 2) - 1});
    SWRender_CopyBufferScaled(&buffer_output, NULL, &buffer_scaled, &(SWRenderRect){ 0, size / 2, (size / 2) - 1, size - 1});
    SWRender_CopyBufferScaled(&buffer_output, NULL, &buffer_scaled, &(SWRenderRect){ size / 2, size / 2, size - 1, size - 1});

    // save
    // EncodeImageAndSaveToFile("./test-sw-render-scale.png", frame_scaled, AV_CODEC_ID_PNG);
    SWRender_SaveImageFromBuffer(&buffer_scaled, "./test-sw-render-scaled.png", AV_CODEC_ID_PNG);

end:
    SWRender_BufferFree(&buffer_image);

    SWRender_BufferFree(&buffer_scaled);
    SWRender_BufferFree(&buffer_output);

    FTEssentials_Done(&ft_state);

    /*
    av_frame_free(&frame_scaled);
    av_frame_free(&frame_output);
    */

    return ret;
}

void ImageSourceTest(void)
{
    for (int i = 0; images_to_test[i]; i++)
    {
        const char *filepath = images_to_test[i];

        printf(LOCATION_PREFIX_STR "transcoding \'%s\'\n", filepath);

        SavePixelsFromFirstFrameOfVideoFileToFileInSameNameWithNewExt(filepath);
    }

    TestSWRender();
}
